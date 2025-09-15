#pragma once
#include <SDL3/SDL.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <stdexcept>

#include "pixel_buffer.hpp"
#include "reactor.hpp"

#define RGB_BLACK 0
#define RGB_WHITE 255

#define RGB_LIGHT_GRAY 180
#define RGB_VOID 21

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

class AReactorWindow {
	SDL_Window *window;
	SDL_Renderer *renderer;
	PixelBuffer *pb;

	Reactor *reactor;

	void draw_bounding_line(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2) {
		thickLineRGBA(renderer, x1, y1, x2, y2, 4, CLR_BLACK, SDL_ALPHA_OPAQUE);
	}

public:

	AReactorWindow(int width, int height, Reactor *rreactor) : reactor(rreactor) {
		SDL_SetAppMetadata("Reactor", "0.1", "com.toy.areactor");

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		if (!SDL_CreateWindowAndRenderer(
				"reactor", width, height, SDL_WindowFlags(0),
				&window, &renderer
		)) {
			SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		pb = new PixelBuffer(renderer, width, height);

		SDL_SetRenderDrawColor(renderer, CLR_WHITE, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	~AReactorWindow() {
		delete pb;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void outline_reactor() {
		Sint16 x = reactor->bbox.x, y = reactor->bbox.y;
		Sint16 w = reactor->bbox.w, h = reactor->bbox.h;
		draw_bounding_line(x, y, x + w, y);
		draw_bounding_line(x, y, x, y + h);
		draw_bounding_line(x + w, y, x + w, y + h);
		draw_bounding_line(x, y + h, x + w, y + h);
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, CLR_WHITE, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}

	void present() {
		SDL_RenderPresent(renderer);
	}
};
