#pragma once
#include <SDL3/SDL.h>
#include <stdexcept>

#include "axes.hpp"
#include "linalg.hpp"
#include "pixel_buffer.hpp"

#define RGB_BLACK 0
#define RGB_WHITE 255

#define RGB_LIGHT_GRAY 180
#define RGB_VOID 18

#define RGB_AMBIENT 26
#define RGB_DIFFUSION 147
#define RGB_SPECULAR 100

#define CLR_MONO(v) v, v, v

#define CLR_BLACK CLR_MONO(RGB_BLACK)
#define CLR_WHITE CLR_MONO(RGB_WHITE)
#define CLR_GRID CLR_MONO(RGB_LIGHT_GRAY)

#define CLR_BG CLR_BLACK

#define CLR_AMBIENT CLR_MONO(RGB_AMBIENT)
#define CLR_VOID CLR_MONO(RGB_VOID)

class DrawWindow {
public:
	SDL_Window *window;
	SDL_Renderer *renderer;
	PixelBuffer *pb;

	DrawWindow(int width, int height) {
		SDL_SetAppMetadata("Example Draww", "0.1", "com.example.draww");

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		if (!SDL_CreateWindowAndRenderer(
				"example", width, height, SDL_WindowFlags(0),
				&window, &renderer
		)) {
			SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		pb = new PixelBuffer(renderer, width, height);

		SDL_SetRenderDrawColor(renderer, CLR_BG, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	~DrawWindow() {
		delete pb;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, CLR_BG, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}

	void present() {
		SDL_RenderPresent(renderer);
	}

	void blit_axes(const CoordinateSystem *cs);
	void blit_grid(const CoordinateSystem *cs);
	void blit_bg(const CoordinateSystem *cs, Uint8 r, Uint8 g, Uint8 b);
	void blit_vector(const CoordinateSystem *cs, Vector2 vec);

	void draw_func(const CoordinateSystem *cs, double (fn)(double));

	void render_sphere_with_ambient_diffusion_and_specular_light(
		const CoordinateSystem *cs,
		const Sphere *sph,
		const Vector3 *light,
		const Vector3 *camera);
};
