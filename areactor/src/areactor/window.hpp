#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <cstdio>
#include <stdexcept>

#include "pixel_buffer.hpp"
#include "reactor.hpp"

// #EAEAEA (234, 234, 234) -- Platinum       (white)
// #000F08 (0,   15,  8  ) -- Night          (black)
// #348AA7 (52,  138, 167) -- Blue           (blue)
// #AA4465 (170, 68,  101) -- Raspberry rose (red)
// #C37D92 (195, 125, 146) -- Puce           (pink)

#define RGB_BLACK 0
#define RGB_WHITE 255

#define CLR_MONO(v) v, v, v

#define CLR_BLACK CLR_MONO(RGB_BLACK)
#define CLR_WHITE CLR_MONO(RGB_WHITE)
#define CLR_GRID CLR_MONO(RGB_LIGHT_GRAY)

#define CLR_BG CLR_BLACK

#define CLR_AMBIENT CLR_MONO(RGB_AMBIENT)
#define CLR_VOID CLR_MONO(RGB_VOID)

#define CLR_PLATINUM 234, 234, 234
#define CLR_NIGHT 0, 15, 8
#define CLR_BLUE 52, 138, 167
#define CLR_RASPBERRY 170, 68,  101
#define CLR_PUCE 195, 125, 146

class AReactorWindow {
	SDL_Window *window;
	SDL_Renderer *renderer;
	PixelBuffer *pb;

	Reactor *reactor;

	void draw_bounding_line(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2) {
		thickLineRGBA(renderer, x1, y1, x2, y2, 4, CLR_NIGHT, SDL_ALPHA_OPAQUE);
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

		clear();
		present();
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

	void draw_particles() {
		std::vector<Slot> &active_slots = reactor->particles->active_slots;
		for (size_t i = 0; i < active_slots.size(); ++i) {
			Slot slot = active_slots[i];
			Particle *p = reactor->particles->items[slot];
			p->draw(renderer, reactor);
		}
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, CLR_PLATINUM, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}

	void present() {
		SDL_RenderPresent(renderer);
	}
};
