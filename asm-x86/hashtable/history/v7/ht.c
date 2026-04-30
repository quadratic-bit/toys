/*
 * ht.c v7
 * store only 32 bits of hash
 */

#include "ht.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

typedef struct ht_entry {
	uint64_t prefix8;
	uint64_t count;

	uint32_t hash;
	uint32_t next;        /* 0 = end of chain */
	uint32_t key_off;     /* offset into arena */
	uint32_t key_len;
} ht_entry_t;

struct ht {
	uint32_t   *buckets;     /* bucket heads, 0 = empty, indices are 1-based */
	size_t      bucket_count;

	ht_entry_t *entries;     /* entries[0] unused */
	size_t      size;        /* number of distinct keys */
	size_t      entries_cap; /* usable entries, excluding slot 0 */

	char       *arena;       /* owns copied key bytes */
	size_t      arena_len;
	size_t      arena_cap;
};

static inline uint64_t mix64(uint64_t x) {
	x ^= x >> 33;
	x *= 0xff51afd7ed558ccdULL;
	x ^= x >> 33;
	x *= 0xc4ceb9fe1a85ec53ULL;
	x ^= x >> 33;
	return x;
}

static uint64_t hash_bytes(const void *data, size_t len) {
	const uint8_t *p = (const uint8_t *)data;
	const size_t orig_len = len;
	uint32_t crc = 0;

	while (len >= 8) {
		uint64_t v;
		memcpy(&v, p, sizeof(v));
		crc = (uint32_t)_mm_crc32_u64(crc, v);
		p += 8;
		len -= 8;
	}

	if (len >= 4) {
		uint32_t v;
		memcpy(&v, p, sizeof(v));
		crc = _mm_crc32_u32(crc, v);
		p += 4;
		len -= 4;
	}
	if (len >= 2) {
		uint16_t v;
		memcpy(&v, p, sizeof(v));
		crc = _mm_crc32_u16(crc, v);
		p += 2;
		len -= 2;
	}
	if (len) {
		crc = _mm_crc32_u8(crc, *p);
	}

	return mix64(((uint64_t)crc << 32) ^ (uint64_t)orig_len);
}

static uint64_t prefix8_bytes(const void *data, size_t len) {
	const unsigned char *p = (const unsigned char *)data;
	size_t n = len < 8 ? len : 8;
	uint64_t v = 0;

	for (size_t i = 0; i < n; i++) {
		v |= (uint64_t)p[i] << (i * 8);
	}

	return v;
}

static inline __attribute__((always_inline))
int key_eq_9_16_asm(const void *a, const void *b, size_t len) {
	assert(len > 8 && len <= 16);

	const char *pa = (const char *)a;
	const char *pb = (const char *)b;
	size_t off = len - 8;
	unsigned char eq;

	__asm__ volatile(
		"movq (%[pa],%[off],1), %%rax\n\t"
		"cmpq (%[pb],%[off],1), %%rax\n\t"
		"sete %[eq]\n\t"
		: [eq] "=q"(eq)
		: [pa] "r"(pa), [pb] "r"(pb), [off] "r"(off)
		: "rax", "cc", "memory"
	);

	return (int)eq;
}

static inline __attribute__((always_inline))
int key_eq_17_32_sse(const void *a, const void *b, size_t len) {
	assert(len > 16 && len <= 32);

	const char *pa = (const char *)a;
	const char *pb = (const char *)b;

	/* tail 16 bytes are always fully inside bounds for len > 16 */
	__m128i tail_a = _mm_loadu_si128((const __m128i *)(pa + len - 16));
	__m128i tail_b = _mm_loadu_si128((const __m128i *)(pb + len - 16));
	__m128i tail_eq = _mm_cmpeq_epi8(tail_a, tail_b);

	if (_mm_movemask_epi8(tail_eq) != 0xFFFF) {
		return 0;
	}

	/*
	 * prefix8 proves [0..7].
	 *
	 * for 17..24:
	 *   prefix8 + tail16 cover the whole key.
	 *
	 * for len 25..32:
	 *   compare bytes [8..23] as well.
	 */
	if (len <= 24) {
		return 1;
	}

	__m128i mid_a = _mm_loadu_si128((const __m128i *)(pa + 8));
	__m128i mid_b = _mm_loadu_si128((const __m128i *)(pb + 8));
	__m128i mid_eq = _mm_cmpeq_epi8(mid_a, mid_b);

	return _mm_movemask_epi8(mid_eq) == 0xFFFF;
}

static inline __attribute__((always_inline))
int key_eq_candidate(const ht_t *ht, const ht_entry_t *e, const void *key, size_t key_len) {
	const char *stored = ht->arena + e->key_off;

	/* prefix8 already proves equality for keys up to 8 bytes */
	if (key_len <= 8) {
		return 1;
	}
	if (key_len <= 16) {
		return key_eq_9_16_asm(stored, key, key_len);
	}
	if (key_len <= 32) {
		return key_eq_17_32_sse(stored, key, key_len);
	}

	return memcmp(stored, key, key_len) == 0;
}

static size_t next_pow2(size_t x) {
	size_t p = 1;

	while (p < x) {
		if (p > (SIZE_MAX >> 1)) return 0;
		p <<= 1;
	}

	return p;
}

static size_t grow(size_t have, size_t needed) {
	while (have < needed) {
		if (have > (SIZE_MAX / 2)) return 0;
		have *= 2;
	}
	return have;
}

static int grow_entries(ht_t *ht, size_t need_entries) {
	size_t new_cap = ht->entries_cap ? ht->entries_cap : 16;

	new_cap = grow(new_cap, need_entries);

	/*                                       slot zero      --v           */
	ht_entry_t *new_entries = realloc(ht->entries, (new_cap + 1) * sizeof(ht_entry_t));
	if (!new_entries) {
		return 0;
	}

	ht->entries = new_entries;
	ht->entries_cap = new_cap;
	return 1;
}

static int grow_arena(ht_t *ht, size_t need_bytes) {
	size_t new_cap = ht->arena_cap ? ht->arena_cap : 4096;

	new_cap = grow(new_cap, need_bytes);

	char *new_arena = (char *)realloc(ht->arena, new_cap);
	if (!new_arena) {
		return 0;
	}

	ht->arena = new_arena;
	ht->arena_cap = new_cap;
	return 1;
}

static int rehash(ht_t *ht, size_t new_bucket_count) {
	uint32_t *new_buckets = calloc(new_bucket_count, sizeof(uint32_t));
	if (!new_buckets) {
		return 0;
	}

	for (uint32_t i = 1; i <= (uint32_t)ht->size; i++) {
		size_t bucket = (size_t)(ht->entries[i].hash & (uint32_t)(new_bucket_count - 1));
		ht->entries[i].next = new_buckets[bucket];
		new_buckets[bucket] = i;
	}

	free(ht->buckets);
	ht->buckets = new_buckets;
	ht->bucket_count = new_bucket_count;
	return 1;
}

ht_t *ht_create() {
	const size_t initial_buckets = 1024;
	const size_t initial_entries = (size_t)((double)initial_buckets * LOAD_FACTOR) + 1;
	const size_t initial_arena   = 64 * 1024;

	ht_t *ht = calloc(1, sizeof(*ht));
	if (!ht) return NULL;

	ht->bucket_count = initial_buckets;
	ht->entries_cap  = initial_entries;
	ht->arena_cap    = initial_arena;

	ht->buckets =   (uint32_t *)calloc(ht->bucket_count, sizeof(uint32_t));
	ht->entries = (ht_entry_t *)malloc((ht->entries_cap + 1) * sizeof(ht_entry_t));
	ht->arena   =       (char *)malloc(ht->arena_cap);

	if (!ht->buckets || !ht->entries || !ht->arena) {
		ht_destroy(ht);
		return NULL;
	}

	return ht;
}

void ht_destroy(ht_t *ht) {
	if (!ht) return;

	free(ht->buckets);
	free(ht->entries);
	free(ht->arena);
	free(ht);
}

void ht_clear(ht_t *ht) {
	if (!ht) return;

	if (ht->buckets && ht->bucket_count) {
		memset(ht->buckets, 0, ht->bucket_count * sizeof(uint32_t));
	}

	ht->size = 0;
	ht->arena_len = 0;
}

size_t ht_size(const ht_t *ht) {
	return ht ? ht->size : 0;
}

size_t ht_bucket_count(const ht_t *ht) {
	return ht ? ht->bucket_count : 0;
}

double ht_load_factor(const ht_t *ht) {
	if (!ht || ht->bucket_count == 0) {
		return 0.0;
	}

	return (double)ht->size / (double)ht->bucket_count;
}

size_t ht_memory_usage(const ht_t *ht) {
	if (!ht) return 0;

	return sizeof(*ht)
		+ ht->bucket_count * sizeof(uint32_t)
		+ (ht->entries_cap + 1) * sizeof(ht_entry_t)
		+ ht->arena_cap;
}

uint64_t ht_increment(ht_t *ht, const void *key, size_t key_len, uint64_t delta) {
	if (!ht || (!key && key_len != 0)) {
		return 0;
	}

	uint64_t hash = hash_bytes(key, key_len);
	uint32_t hash32 = (uint32_t)hash;
	uint64_t prefix8 = prefix8_bytes(key, key_len);
	size_t bucket = (size_t)(hash32 & (uint32_t)(ht->bucket_count - 1));

	for (uint32_t i = ht->buckets[bucket]; i != 0; i = ht->entries[i].next) {
		ht_entry_t *e = &ht->entries[i];

		if (e->hash == hash32 && e->key_len == key_len && e->prefix8 == prefix8 &&
		    key_eq_candidate(ht, e, key, key_len)
		) {
			e->count += delta;
			return e->count;
		}
	}

	/* no entry found: insert */

	if ((double)(ht->size + 1) > (double)ht->bucket_count * LOAD_FACTOR) {
		size_t new_bucket_count = next_pow2(ht->bucket_count ? ht->bucket_count * 2 : 1024);
		if (new_bucket_count == 0 || !rehash(ht, new_bucket_count)) {
			return 0;
		}
		bucket = (size_t)(hash32 & (uint32_t)(ht->bucket_count - 1));
	}

	if (ht->size + 1 > ht->entries_cap) {
		if (!grow_entries(ht, ht->size + 1)) {
			return 0;
		}
	}

	if (ht->arena_len + key_len > ht->arena_cap) {
		if (!grow_arena(ht, ht->arena_len + key_len)) {
			return 0;
		}
	}

	uint32_t key_off = (uint32_t)ht->arena_len;
	if (key_len != 0) {
		memcpy(ht->arena + ht->arena_len, key, key_len);
		ht->arena_len += key_len;
	}

	uint32_t idx = (uint32_t)(ht->size + 1);
	ht_entry_t *e = &ht->entries[idx];

	e->prefix8  = prefix8;
	e->count    = delta;
	e->hash     = hash32;
	e->next     = ht->buckets[bucket];
	e->key_off  = key_off;
	e->key_len  = (uint32_t)key_len;

	ht->buckets[bucket] = idx;
	ht->size = idx;

	return e->count;
}

uint64_t ht_lookup(const ht_t *ht, const void *key, size_t key_len) {
	if (!ht || (!key && key_len != 0) || ht->bucket_count == 0) {
		return 0;
	}

	uint64_t hash = hash_bytes(key, key_len);
	uint32_t hash32 = (uint32_t)hash;
	uint64_t prefix8 = prefix8_bytes(key, key_len);
	size_t bucket = (size_t)(hash32 & (uint32_t)(ht->bucket_count - 1));

	for (uint32_t i = ht->buckets[bucket]; i != 0; i = ht->entries[i].next) {
		const ht_entry_t *e = &ht->entries[i];

		if (e->hash == hash32 && e->key_len == key_len && e->prefix8 == prefix8 &&
		    key_eq_candidate(ht, e, key, key_len)
		) {
			return e->count;
		}
	}

	return 0;
}

ht_view_t ht_most_frequent(const ht_t *ht) {
	ht_view_t out;
	out.key.ptr = NULL;
	out.key.len = 0;
	out.count   = 0;

	if (!ht || ht->size == 0) {
		return out;
	}

	size_t best = 1;
	for (size_t i = 2; i <= ht->size; i++) {
		if (ht->entries[i].count > ht->entries[best].count) {
			best = i;
		}
	}

	out.key.ptr = ht->arena + ht->entries[best].key_off;
	out.key.len = ht->entries[best].key_len;
	out.count   = ht->entries[best].count;
	return out;
}
