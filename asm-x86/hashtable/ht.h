#ifndef HT_H_INCLUDED
#define HT_H_INCLUDED

#include <stddef.h>
#include <stdint.h>

typedef struct ht ht_t;

#define LOAD_FACTOR 2.1

typedef struct ht_bytes {
	const char *ptr;
	size_t      len;
} ht_bytes_t;

/*
 * View into an entry stored inside the table.
 * Key pointer owned by the table.
 */
typedef struct ht_view {
	ht_bytes_t key;
	uint64_t   count;
} ht_view_t;

ht_t *ht_create(void);
void  ht_destroy(ht_t *ht);

/*
 * Remove all entries but keep allocated memory/capacity for reuse.
 */
void ht_clear(ht_t *ht);

/* Distinct-key count. */
size_t ht_size(const ht_t *ht);

/* Introspection helpers. */
size_t ht_bucket_count(const ht_t *ht);
double ht_load_factor (const ht_t *ht);
size_t ht_memory_usage(const ht_t *ht);

uint64_t ht_increment(ht_t *ht, const void *key, size_t key_len, uint64_t delta);
uint64_t ht_lookup(const ht_t *ht, const void *key, size_t key_len);

/*
 * Return one maximal-count entry.
 * On empty table, returns view with an empty pointer.
 */
ht_view_t ht_most_frequent(const ht_t *ht);

#endif /* HT_H_INCLUDED */
