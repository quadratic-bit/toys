#define _POSIX_C_SOURCE 200809L

#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include <immintrin.h>
#include <x86intrin.h>

#define SCREEN_WIDTH  900
#define SCREEN_HEIGHT 600
#define ITERATION_LIMIT 256

#ifndef BENCH_FRAMES
#define BENCH_FRAMES 3000
#endif

#if defined(BENCH_ZERO_DRAW)
#define BENCH_MODE_STR "zero"
#else
#define BENCH_MODE_STR "mandelbrot"
#endif

typedef struct {
	unsigned char r, g, b, a;
} RGBA8;

static RGBA8 g_colors[ITERATION_LIMIT + 1];

static inline uint64_t wall_now_ns(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

static inline uint64_t cycles_now_begin(void) {
	_mm_lfence();
	return __rdtsc();
}

static inline uint64_t cycles_now_end(void) {
	unsigned int aux = 0;
	uint64_t t = __rdtscp(&aux);
	_mm_lfence();
	return t;
}

static void init_colors(void) {
	for (int i = 0; i < ITERATION_LIMIT; ++i) {
		float t = (float)i / (float)(ITERATION_LIMIT - 1);

		float r = 9.0f  * (1.0f - t) * t * t * t;
		float g = 15.0f * (1.0f - t) * (1.0f - t) * t * t;
		float b = 8.5f  * (1.0f - t) * (1.0f - t) * (1.0f - t) * t;

		if (r > 1.0f) r = 1.0f;
		if (g > 1.0f) g = 1.0f;
		if (b > 1.0f) b = 1.0f;

		g_colors[i].r = (unsigned char)(255.0f * r);
		g_colors[i].g = (unsigned char)(255.0f * g);
		g_colors[i].b = (unsigned char)(255.0f * b);
		g_colors[i].a = 255;
	}

	g_colors[ITERATION_LIMIT].r = 0;
	g_colors[ITERATION_LIMIT].g = 0;
	g_colors[ITERATION_LIMIT].b = 0;
	g_colors[ITERATION_LIMIT].a = 255;
}

static inline uint32_t pack_rgba(RGBA8 c) {
	return ((uint32_t)c.r)
		| ((uint32_t)c.g << 8)
		| ((uint32_t)c.b << 16)
		| ((uint32_t)c.a << 24);
}

static inline RGBA8 lerp_color(RGBA8 a, RGBA8 b, float t) {
	RGBA8 c;
	c.r = (unsigned char)(a.r + (b.r - a.r) * t);
	c.g = (unsigned char)(a.g + (b.g - a.g) * t);
	c.b = (unsigned char)(a.b + (b.b - a.b) * t);
	c.a = (unsigned char)(a.a + (b.a - a.a) * t);
	return c;
}

static inline uint32_t color_from_smooth(float smooth) {
	if (smooth >= (float)ITERATION_LIMIT) {
		return pack_rgba(g_colors[ITERATION_LIMIT]);
	}

	int i0 = (int)floorf(smooth);
	int i1 = i0 + 1;
	if (i1 >= ITERATION_LIMIT) {
		i1 = ITERATION_LIMIT - 1;
	}

	float t = smooth - (float)i0;
	return pack_rgba(lerp_color(g_colors[i0], g_colors[i1], t));
}

static inline uint32_t mandelbrot_pixel_scalar(float c_x, float c_y) {
	float x = 0.0f;
	float y = 0.0f;

	for (int step = 0; step < ITERATION_LIMIT; ++step) {
		float x_sq = x * x;
		float y_sq = y * y;
		float mag_sq = x_sq + y_sq;

		if (mag_sq > 4.0f) {
			float smooth = (float)step + 1.0f - log2f(logf(sqrtf(mag_sq)));
			return color_from_smooth(smooth);
		}

		float new_y = 2.0f * x * y + c_y;
		float new_x = x_sq - y_sq + c_x;
		x = new_x;
		y = new_y;
	}

	return pack_rgba(g_colors[ITERATION_LIMIT]);
}

static inline void mandelbrot8_avx2(__m256 c_x, __m256 c_y, uint32_t *out_pixels) {
	const __m256 zero = _mm256_setzero_ps();
	const __m256 two  = _mm256_set1_ps(2.0f);
	const __m256 four = _mm256_set1_ps(4.0f);

	const __m256i all_ones = _mm256_set1_epi32(-1);

	__m256 x = zero;
	__m256 y = zero;

	__m256 escape_mag = zero;
	__m256i escape_step = _mm256_set1_epi32(-1);
	__m256i escaped = _mm256_setzero_si256();

	for (int step = 0; step < ITERATION_LIMIT; ++step) {
		__m256 x_sq = _mm256_mul_ps(x, x);
		__m256 y_sq = _mm256_mul_ps(y, y);
		__m256 mag_sq = _mm256_add_ps(x_sq, y_sq);

		__m256 gt4_ps = _mm256_cmp_ps(mag_sq, four, _CMP_GT_OQ);
		__m256i gt4 = _mm256_castps_si256(gt4_ps);

		__m256i escaped_now = _mm256_andnot_si256(escaped, gt4);

		escape_mag = _mm256_blendv_ps(escape_mag, mag_sq, _mm256_castsi256_ps(escaped_now));

		__m256i step_vec = _mm256_set1_epi32(step);
		escape_step = _mm256_blendv_epi8(escape_step, step_vec, escaped_now);

		escaped = _mm256_or_si256(escaped, escaped_now);

		__m256i active = _mm256_xor_si256(escaped, all_ones);
		if (_mm256_testz_si256(active, active)) {
			break;
		}

		__m256 xy = _mm256_mul_ps(x, y);
		__m256 new_y = _mm256_fmadd_ps(two, xy, c_y);
		__m256 new_x = _mm256_add_ps(_mm256_sub_ps(x_sq, y_sq), c_x);

		__m256 active_ps = _mm256_castsi256_ps(active);
		x = _mm256_blendv_ps(x, new_x, active_ps);
		y = _mm256_blendv_ps(y, new_y, active_ps);
	}

	float mags[8];
	int steps[8];

	_mm256_storeu_ps(mags, escape_mag);
	_mm256_storeu_si256((__m256i *)steps, (__m256i)escape_step);

	for (int lane = 0; lane < 8; ++lane) {
		if (steps[lane] < 0) {
			out_pixels[lane] = pack_rgba(g_colors[ITERATION_LIMIT]);
		} else {
			float smooth = (float)steps[lane] + 1.0f - log2f(logf(sqrtf(mags[lane])));
			out_pixels[lane] = color_from_smooth(smooth);
		}
	}
}

static void render_frame(uint32_t *pixels) {
#if defined(BENCH_ZERO_DRAW)
	memset(pixels, 0, (size_t)SCREEN_WIDTH * (size_t)SCREEN_HEIGHT * sizeof(uint32_t));
#else
	const float origin_x = -0.5f;
	const float origin_y = 0.0f;
	const float zoom = 1.0f;

	const float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	const float real_min = origin_x - aspect / zoom;
	const float real_max = origin_x + aspect / zoom;
	const float imag_min = origin_y - 1.0f / zoom;
	const float imag_max = origin_y + 1.0f / zoom;

	const float dx = (real_max - real_min) / (float)(SCREEN_WIDTH - 1);
	const float dy = (imag_max - imag_min) / (float)(SCREEN_HEIGHT - 1);

	const __m256 lane_offsets = _mm256_setr_ps(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);
	const __m256 dx_vec = _mm256_set1_ps(dx);

	const int simd_width = 8;
	const int simd_limit = SCREEN_WIDTH & ~(simd_width - 1);

	for (int y = 0; y < SCREEN_HEIGHT; ++y) {
		const int row = y * SCREEN_WIDTH;
		const float c_y_scalar = imag_max - (float)y * dy;
		const __m256 c_y = _mm256_set1_ps(c_y_scalar);

		int x = 0;
		for (; x < simd_limit; x += simd_width) {
			const float base_x = real_min + (float)x * dx;
			const __m256 c_x = _mm256_add_ps(_mm256_set1_ps(base_x), _mm256_mul_ps(lane_offsets, dx_vec));
			mandelbrot8_avx2(c_x, c_y, &pixels[row + x]);
		}

		for (; x < SCREEN_WIDTH; ++x) {
			const float c_x_scalar = real_min + (float)x * dx;
			pixels[row + x] = mandelbrot_pixel_scalar(c_x_scalar, c_y_scalar);
		}
	}
#endif
}

int main(void) {
	const size_t pixel_count = (size_t)SCREEN_WIDTH * (size_t)SCREEN_HEIGHT;

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "mandelbrot avx2");

	ClearWindowState(FLAG_VSYNC_HINT);
	SetTargetFPS(0);
	DisableEventWaiting();

	uint32_t *pixels = calloc(pixel_count, sizeof(uint32_t));
	if (!pixels) {
		CloseWindow();
		return 1;
	}

	Image image = {
		.data = pixels,
		.width = SCREEN_WIDTH,
		.height = SCREEN_HEIGHT,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
	};

	Texture2D texture = LoadTextureFromImage(image);
	SetTextureFilter(texture, TEXTURE_FILTER_POINT);

	init_colors();

#if defined(BENCH_PRINT)
	uint64_t render_wall_ns_total = 0;
	uint64_t render_cycles_total = 0;

	const uint64_t bench_wall_ns_begin = wall_now_ns();
	const uint64_t bench_cycles_begin = cycles_now_begin();
#endif

	int frames_done = 0;
	for (; frames_done < BENCH_FRAMES; ++frames_done) {
		if (WindowShouldClose()) {
			break;
		}

#if defined(BENCH_PRINT)
		const uint64_t render_wall_ns_begin = wall_now_ns();
		const uint64_t render_cycles_begin = cycles_now_begin();
#endif

		render_frame(pixels);

#if defined(BENCH_PRINT)
		const uint64_t render_cycles_end = cycles_now_end();
		const uint64_t render_wall_ns_end = wall_now_ns();

		render_wall_ns_total += render_wall_ns_end - render_wall_ns_begin;
		render_cycles_total += render_cycles_end - render_cycles_begin;
#endif

		UpdateTexture(texture, pixels);

		BeginDrawing();
		DrawTexture(texture, 0, 0, WHITE);
		EndDrawing();
	}

#if defined(BENCH_PRINT)
	const uint64_t bench_cycles_end = cycles_now_end();
	const uint64_t bench_wall_ns_end = wall_now_ns();

	const uint64_t bench_wall_ns_total = bench_wall_ns_end - bench_wall_ns_begin;
	const uint64_t bench_cycles_total = bench_cycles_end - bench_cycles_begin;

	const long double frames_ld = (frames_done > 0) ? (long double)frames_done : 1.0L;
	const long double bench_wall_s = (long double)bench_wall_ns_total / 1000000000.0L;
	const long double render_wall_s = (long double)render_wall_ns_total / 1000000000.0L;
	const long double bench_wall_per_frame_ns = (long double)bench_wall_ns_total / frames_ld;
	const long double render_wall_per_frame_ns = (long double)render_wall_ns_total / frames_ld;
	const long double bench_cycles_per_frame = (long double)bench_cycles_total / frames_ld;
	const long double render_cycles_per_frame = (long double)render_cycles_total / frames_ld;

	printf(
		"impl=avx2 mode=%s frames=%d sample=%08" PRIx32
		" vsync_hint=%d"
		" bench_wall_ns=%" PRIu64
		" bench_wall_s=%.18Lf"
		" bench_wall_per_frame_ns=%.18Lf"
		" render_wall_ns=%" PRIu64
		" render_wall_s=%.18Lf"
		" render_wall_per_frame_ns=%.18Lf"
		" bench_cycles=%" PRIu64
		" bench_cycles_per_frame=%.18Lf"
		" render_cycles=%" PRIu64
		" render_cycles_per_frame=%.18Lf\n",
		BENCH_MODE_STR,
		frames_done,
		pixels[pixel_count / 2],
		IsWindowState(FLAG_VSYNC_HINT) ? 1 : 0,
		bench_wall_ns_total,
		bench_wall_s,
		bench_wall_per_frame_ns,
		render_wall_ns_total,
		render_wall_s,
		render_wall_per_frame_ns,
		bench_cycles_total,
		bench_cycles_per_frame,
		render_cycles_total,
		render_cycles_per_frame
	);
#endif

	UnloadTexture(texture);
	free(pixels);
	CloseWindow();
	return 0;
}
