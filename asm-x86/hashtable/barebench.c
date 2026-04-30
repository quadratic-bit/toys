#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <valgrind/callgrind.h>

#include <x86intrin.h>

typedef struct loaded_file {
	char       *data;
	size_t      size;
	const char *path;
} loaded_file_t;

typedef struct phase_sample {
	struct timespec time_start;
	struct timespec time_end;
	uint64_t        cycle_start;
	uint64_t        cycle_end;
} phase_sample_t;

static volatile uint64_t g_token_sink = 0;

static void die_perror(const char *what, const char *path) {
	if (path != NULL) {
		fprintf(stderr, "error: %s '%s': %s\n", what, path, strerror(errno));
	} else {
		fprintf(stderr, "error: %s: %s\n", what, strerror(errno));
	}
	exit(EXIT_FAILURE);
}

static void die_msg(const char *msg) {
	fprintf(stderr, "error: %s\n", msg);
	exit(EXIT_FAILURE);
}

static clockid_t bench_clock_id(void) {
	return CLOCK_MONOTONIC_RAW;
}

static uint64_t rdtsc_begin(void) {
	_mm_lfence();
	return __rdtsc();
}

static uint64_t rdtsc_end(void) {
	unsigned int aux = 0;
	const uint64_t t = __rdtscp(&aux);
	_mm_lfence();
	return t;
}

static void sample_begin(phase_sample_t *sample) {
	if (clock_gettime(bench_clock_id(), &sample->time_start) != 0) {
		die_perror("clock_gettime", NULL);
	}
	sample->cycle_start = rdtsc_begin();
}

static void sample_end(phase_sample_t *sample) {
	sample->cycle_end = rdtsc_end();
	if (clock_gettime(bench_clock_id(), &sample->time_end) != 0) {
		die_perror("clock_gettime", NULL);
	}
}

static uint64_t timespec_diff_ns(const struct timespec *start, const struct timespec *end) {
	const int64_t sec  = (int64_t)end->tv_sec  - (int64_t)start->tv_sec;
	const int64_t nsec = (int64_t)end->tv_nsec - (int64_t)start->tv_nsec;
	return (uint64_t)(sec * 1000000000LL + nsec);
}

static uint64_t sample_ns(const phase_sample_t *sample) {
	return timespec_diff_ns(&sample->time_start, &sample->time_end);
}

static uint64_t sample_cycles(const phase_sample_t *sample) {
	return sample->cycle_end - sample->cycle_start;
}

static void format_human_time(uint64_t ns, char *buf, size_t buf_size) {
	const double value = (double)ns;

	if (ns < 1000ULL)
		(void)snprintf(buf, buf_size, "%.0f ns", value);
	else if (ns < 1000000ULL)
		(void)snprintf(buf, buf_size, "%.3f us", value / 1e3);
	else if (ns < 1000000000ULL)
		(void)snprintf(buf, buf_size, "%.3f ms", value / 1e6);
	else if (ns < 60000000000ULL)
		(void)snprintf(buf, buf_size, "%.6f s", value / 1e9);
	else {
		const double sec = value / 1e9;
		const unsigned long long mins = (unsigned long long)(sec / 60.0);
		(void)snprintf(buf, buf_size, "%llumin %.3fs", mins, sec - 60.0 * (double)mins);
	}
}

static void format_human_cycles(uint64_t cycles, char *buf, size_t buf_size) {
	const double value = (double)cycles;

	if (cycles < 1000ULL) {
		(void)snprintf(buf, buf_size, "%.0f cyc", value);
	} else if (cycles < 1000000ULL) {
		(void)snprintf(buf, buf_size, "%.3f Kcyc", value / 1e3);
	} else if (cycles < 1000000000ULL) {
		(void)snprintf(buf, buf_size, "%.3f Mcyc", value / 1e6);
	} else {
		(void)snprintf(buf, buf_size, "%.3f Gcyc", value / 1e9);
	}
}

static void print_phase(const char *name, const phase_sample_t *sample) {
	char wall_human[64];
	char cyc_human[64];
	const uint64_t wall_ns = sample_ns(sample);
	const uint64_t cycles = sample_cycles(sample);

	format_human_time(wall_ns, wall_human, sizeof(wall_human));
	format_human_cycles(cycles, cyc_human, sizeof(cyc_human));

	printf("%s: %lu ns (%s), %lu cycles (%s)\n", name, wall_ns, wall_human, cycles, cyc_human);
}

static loaded_file_t load_file(const char *path) {
	loaded_file_t file = {0};
	int fd = -1;
	struct stat st;
	size_t done = 0;

	fd = open(path, O_RDONLY);
	if (fd < 0) die_perror("open", path);

	if (fstat(fd, &st) != 0) {
		close(fd);
		die_perror("fstat", path);
	}
	if (!S_ISREG(st.st_mode)) {
		close(fd);
		die_msg("input must be a regular file");
	}
	if (st.st_size < 0) {
		close(fd);
		die_msg("negative file size");
	}
	if ((uintmax_t)st.st_size > (uintmax_t)(SIZE_MAX - 1U)) {
		close(fd);
		die_msg("file too large for this build");
	}

	file.size = (size_t)st.st_size;
	file.path = path;
	file.data = (char *)malloc(file.size + 1U);
	if (file.data == NULL) {
		close(fd);
		die_perror("malloc", path);
	}

	while (done < file.size) {
		const ssize_t n = read(fd, file.data + done, file.size - done);
		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			free(file.data);
			close(fd);
			die_perror("read", path);
		}
		if (n == 0) {
			free(file.data);
			close(fd);
			die_msg("unexpected EOF while reading file");
		}
		done += (size_t)n;
	}

	file.data[file.size] = '\0';

	if (close(fd) != 0) {
		free(file.data);
		die_perror("close", path);
	}

	return file;
}

static void free_loaded_file(loaded_file_t *file) {
	free(file->data);
	file->data = NULL;
	file->size = 0;
	file->path = NULL;
}

static int is_ascii_space(unsigned char c) {
	return (c == ' ') || (c >= '\t' && c <= '\r');
}

static unsigned char ascii_tolower_byte(unsigned char c) {
	if (c >= 'A' && c <= 'Z') {
		return (unsigned char)(c + ('a' - 'A'));
	}
	return c;
}

static uint64_t run_insert_phase(loaded_file_t *file) {
	size_t i = 0;
	uint64_t token_count = 0;
	uint64_t sink = 0;

	while (i < file->size) {
		while (i < file->size && is_ascii_space((unsigned char)file->data[i])) {
			++i;
		}

		const size_t start = i;
		while (i < file->size && !is_ascii_space((unsigned char)file->data[i])) {
			file->data[i] = (char)ascii_tolower_byte((unsigned char)file->data[i]);
			++i;
		}

		if (i > start) {
			sink ^= ((uint64_t)(i - start) << 32) ^ (uint64_t)(unsigned char)file->data[start];
			++token_count;
		}
	}

	g_token_sink ^= sink;
	return token_count;
}

static uint64_t run_lookup_phase(loaded_file_t *file) {
	size_t i = 0;
	uint64_t token_count = 0;
	uint64_t sink = 0;

	while (i < file->size) {
		while (i < file->size && is_ascii_space((unsigned char)file->data[i])) {
			++i;
		}

		const size_t start = i;
		while (i < file->size && !is_ascii_space((unsigned char)file->data[i])) {
			file->data[i] = (char)ascii_tolower_byte((unsigned char)file->data[i]);
			++i;
		}

		if (i > start) {
			sink ^= ((uint64_t)(i - start) << 32) ^ (uint64_t)(unsigned char)file->data[start];
			++token_count;
		}
	}

	g_token_sink ^= sink;
	return token_count;
}

int main(int argc, char **argv) {
	loaded_file_t insert_file = {0};
	loaded_file_t lookup_file = {0};
	phase_sample_t insert_sample = {0};
	phase_sample_t lookup_sample = {0};
	uint64_t insert_count = 0;
	uint64_t lookup_count = 0;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <insert-file> <lookup-file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	insert_file = load_file(argv[1]);
	lookup_file = load_file(argv[2]);

	CALLGRIND_START_INSTRUMENTATION;

	sample_begin(&insert_sample);
	insert_count = run_insert_phase(&insert_file);
	sample_end(&insert_sample);

	sample_begin(&lookup_sample);
	lookup_count = run_lookup_phase(&lookup_file);
	sample_end(&lookup_sample);

	CALLGRIND_STOP_INSTRUMENTATION;

	printf("inserts: %lu\n", insert_count);
	printf("lookups: %lu\n", lookup_count);
	printf("token_sink: %" PRIu64 "\n", g_token_sink);
	print_phase("insert", &insert_sample);
	print_phase("lookup", &lookup_sample);

	free_loaded_file(&insert_file);
	free_loaded_file(&lookup_file);

	return EXIT_SUCCESS;
}
