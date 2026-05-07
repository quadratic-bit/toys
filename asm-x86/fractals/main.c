#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH  900
#define SCREEN_HEIGHT 600

#define ITERATION_LIMIT 256
#define TRAJECTORY_CAPACITY (ITERATION_LIMIT + 1)

typedef struct {
	float x;
	float y;
} ComplexPoint;

static Color g_colors[ITERATION_LIMIT + 1];

static void init_colors(void) {
	for (int i = 0; i < ITERATION_LIMIT; ++i) {
		float t = (float)i / (float)(ITERATION_LIMIT - 1);

		float r = 9.0f  * (1.0f - t) * t * t * t;
		float g = 15.0f * (1.0f - t) * (1.0f - t) * t * t;
		float b = 8.5f  * (1.0f - t) * (1.0f - t) * (1.0f - t) * t;

		if (r > 1.0f) r = 1.0f;
		if (g > 1.0f) g = 1.0f;
		if (b > 1.0f) b = 1.0f;

		g_colors[i] = (Color){
			(unsigned char)(255.0f * r),
			(unsigned char)(255.0f * g),
			(unsigned char)(255.0f * b),
			255
		};
	}

	g_colors[ITERATION_LIMIT] = BLACK;
}

static inline float clampf(float x, float lo, float hi) {
	return (x < lo) ? lo : (x > hi) ? hi : x;
}

static inline Color lerp_color(Color a, Color b, float t) {
	t = clampf(t, 0.0f, 1.0f);

	Color c;
	c.r = (unsigned char)(a.r + (b.r - a.r) * t);
	c.g = (unsigned char)(a.g + (b.g - a.g) * t);
	c.b = (unsigned char)(a.b + (b.b - a.b) * t);
	c.a = (unsigned char)(a.a + (b.a - a.a) * t);
	return c;
}

static inline void screen_to_complex(int px, int py, float origin_x, float origin_y, float zoom, float *out_x, float *out_y) {
	float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	float nx = (float)px / (float)(SCREEN_WIDTH - 1);
	float ny = (float)py / (float)(SCREEN_HEIGHT - 1);

	*out_x = ((nx * 2.0f) - 1.0f) * aspect / zoom + origin_x;
	*out_y = (1.0f - (ny * 2.0f)) / zoom + origin_y;
}

static inline Vector2 complex_to_screen(float cx, float cy, float origin_x, float origin_y, float zoom) {
	float aspect = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	float sx = ((cx - origin_x) * zoom / aspect + 1.0f) * 0.5f * (float)(SCREEN_WIDTH - 1);
	float sy = (1.0f - (cy - origin_y) * zoom) * 0.5f * (float)(SCREEN_HEIGHT - 1);

	return (Vector2){ sx, sy };
}

static inline float mandelbrot_smooth(float c_x, float c_y) {
	float x = 0.0f;
	float y = 0.0f;

	for (int step = 0; step < ITERATION_LIMIT; ++step) {
		float x_sq = x * x;
		float y_sq = y * y;
		float mag_sq = x_sq + y_sq;

		if (mag_sq > 4.0f) {
			float smooth = (float)step + 1.0f - log2f(logf(sqrtf(mag_sq)));
			return smooth;
		}

		float new_y = 2.0f * x * y + c_y;
		float new_x = x_sq - y_sq + c_x;
		x = new_x;
		y = new_y;
	}

	return (float)ITERATION_LIMIT;
}

static inline int compute_trajectory(float c_x, float c_y, ComplexPoint *trajectory) {
	float x = 0.0f;
	float y = 0.0f;
	int count = 0;

	trajectory[count++] = (ComplexPoint){ x, y };

	for (int step = 0; step < ITERATION_LIMIT; ++step) {
		float x_sq = x * x;
		float y_sq = y * y;
		float mag_sq = x_sq + y_sq;

		if (mag_sq > 4.0f) {
			break;
		}

		float new_y = 2.0f * x * y + c_y;
		float new_x = x_sq - y_sq + c_x;

		x = new_x;
		y = new_y;

		if (count < TRAJECTORY_CAPACITY) {
			trajectory[count++] = (ComplexPoint){ x, y };
		}
	}

	return count;
}

static inline Color sample_pixel(int px, int py, float origin_x, float origin_y, float zoom) {
	float fx, fy;
	screen_to_complex(px, py, origin_x, origin_y, zoom, &fx, &fy);

	float smooth = mandelbrot_smooth(fx, fy);

	if (smooth >= (float)ITERATION_LIMIT) {
		return g_colors[ITERATION_LIMIT];
	}

	int i0 = (int)floorf(smooth);
	int i1 = i0 + 1;
	if (i1 >= ITERATION_LIMIT) i1 = ITERATION_LIMIT - 1;

	float t = smooth - (float)i0;
	return lerp_color(g_colors[i0], g_colors[i1], t);
}

int main(void) {
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "raylib custom pixels");
	SetTargetFPS(60);

	uint32_t *pixels = malloc((size_t)SCREEN_WIDTH * (size_t)SCREEN_HEIGHT * sizeof(uint32_t));
	if (!pixels) {
		CloseWindow();
		return 1;
	}

	Image image = {
		.data    = pixels,
		.width   = SCREEN_WIDTH,
		.height  = SCREEN_HEIGHT,
		.mipmaps = 1,
		.format  = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
	};

	Texture2D texture = LoadTextureFromImage(image);
	SetTextureFilter(texture, TEXTURE_FILTER_POINT);

	init_colors();

	float origin_x = -0.5f;
	float origin_y = 0.0f;
	float zoom = 1.0f;

	ComplexPoint trajectory[TRAJECTORY_CAPACITY];
	int trajectory_count = 0;

	while (!WindowShouldClose()) {
		float move_step = 0.05f / zoom;

		if (IsKeyDown(KEY_LEFT))  origin_x -= move_step;
		if (IsKeyDown(KEY_RIGHT)) origin_x += move_step;
		if (IsKeyDown(KEY_UP))    origin_y += move_step;
		if (IsKeyDown(KEY_DOWN))  origin_y -= move_step;

		if (IsKeyDown(KEY_J)) zoom *= 1.02f;
		if (IsKeyDown(KEY_K)) zoom /= 1.02f;

		if (zoom < 0.1f) zoom = 0.1f;

		for (int y = 0; y < SCREEN_HEIGHT; ++y) {
			int row = y * SCREEN_WIDTH;
			for (int x = 0; x < SCREEN_WIDTH; ++x) {
				Color c = sample_pixel(x, y, origin_x, origin_y, zoom);

				pixels[row + x] =
					  ((uint32_t)c.r)
					| ((uint32_t)c.g << 8)
					| ((uint32_t)c.b << 16)
					| ((uint32_t)c.a << 24);
			}
		}

		int mouse_x = GetMouseX();
		int mouse_y = GetMouseY();

		float c_x, c_y;
		screen_to_complex(mouse_x, mouse_y, origin_x, origin_y, zoom, &c_x, &c_y);
		trajectory_count = compute_trajectory(c_x, c_y, trajectory);

		UpdateTexture(texture, pixels);

		BeginDrawing();
		ClearBackground(BLACK);
		DrawTexture(texture, 0, 0, WHITE);

		for (int i = 0; i + 1 < trajectory_count; ++i) {
			Vector2 a = complex_to_screen(trajectory[i].x, trajectory[i].y, origin_x, origin_y, zoom);
			Vector2 b = complex_to_screen(trajectory[i + 1].x, trajectory[i + 1].y, origin_x, origin_y, zoom);
			DrawLineV(a, b, RED);
		}

		DrawCircle(mouse_x, mouse_y, 3.0f, RED);
		DrawFPS(10, 10);
		DrawText(TextFormat("origin=(%.6f, %.6f) zoom=%.3f", origin_x, origin_y, zoom), 10, 30, 20, WHITE);
		DrawText(TextFormat("mouse c=(%.6f, %.6f) traj=%d", c_x, c_y, trajectory_count), 10, 55, 20, WHITE);
		EndDrawing();
	}

	UnloadTexture(texture);
	free(pixels);
	CloseWindow();
	return 0;
}
