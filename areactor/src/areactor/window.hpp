#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cstdio>
#include <stdexcept>

#include "linalg/axes.hpp"
#include "pixel_buffer.hpp"
#include "reactor.hpp"
#include "ring_buffer.hpp"

#define RGB_BLACK 0
#define RGB_WHITE 255
#define RGB_LIGHT_GRAY 180

#define CLR_MONO(v) v, v, v

#define CLR_BLACK CLR_MONO(RGB_BLACK)
#define CLR_WHITE CLR_MONO(RGB_WHITE)
#define CLR_GRID CLR_MONO(RGB_LIGHT_GRAY)

#define CLR_BG CLR_BLACK

#define CLR_AMBIENT CLR_MONO(RGB_AMBIENT)
#define CLR_VOID CLR_MONO(RGB_VOID)

// #EAEAEA (234, 234, 234) -- Platinum       (white)
// #000F08 (0,   15,  8  ) -- Night          (black)
// #348AA7 (52,  138, 167) -- Blue           (blue)
// #AA4465 (170, 68,  101) -- Raspberry rose (red)
// #C37D92 (195, 125, 146) -- Puce           (pink)

#define CLR_PLATINUM 234, 234, 234
#define CLR_NIGHT 0, 15, 8
#define CLR_BLUE 52, 138, 167
#define CLR_RASPBERRY 170, 68,  101
#define CLR_PUCE 195, 125, 146

class AReactorWindow {
	SDL_Window *window;
	SDL_Renderer *renderer;
	TTF_TextEngine *text_engine;
	TTF_Font *font;
	PixelBuffer *pb;

	Reactor *reactor;

	void draw_bounding_line(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, unsigned thick) {
		thickLineRGBA(renderer, x1, y1, x2, y2, thick, CLR_NIGHT, SDL_ALPHA_OPAQUE);
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

		if (!TTF_Init()) {
			SDL_Log("Couldn't initialise SDL_ttf: %s\n", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		text_engine = TTF_CreateRendererTextEngine(renderer);
		font = TTF_OpenFont("/usr/share/fonts/TTF/CaskaydiaCoveNerdFontMono-Regular.ttf", 16);
		if (font == NULL) {
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

	void outline(const SDL_FRect box, unsigned thick = 4) {
		Sint16 x = box.x, y = box.y;
		Sint16 w = box.w, h = box.h;
		draw_bounding_line(x, y, x + w, y, thick);
		draw_bounding_line(x, y, x, y + h, thick);
		draw_bounding_line(x + w, y, x + w, y + h, thick);
		draw_bounding_line(x, y + h, x + w, y + h, thick);
	}

	void draw_particles() {
		std::vector<Slot> &active_slots = reactor->particles->active_slots;
		for (size_t i = 0; i < active_slots.size(); ++i) {
			Slot slot = active_slots[i];
			Particle *p = reactor->particles->items[slot];
			p->draw(renderer, reactor);
		}
	}

	void draw_view_grid(const View *view, double units) {
		int grid_offset_x, grid_offset_y, i;

		grid_offset_x = pos_mod(view->y_axis.center - view->dim.x, view->x_axis.scale);
		grid_offset_y = pos_mod(view->x_axis.center - view->dim.y, view->x_axis.scale);

		SDL_SetRenderDrawColor(renderer, CLR_GRID, SDL_ALPHA_OPAQUE);
		for (i = grid_offset_x; i <= view->dim.w; i += view->x_axis.scale * units) {
			SDL_RenderLine(renderer, view->dim.x + i, view->dim.y, view->dim.x + i, view->dim.y + view->dim.h);
		}

		for (i = grid_offset_y; i <= view->dim.h; i += view->x_axis.scale * units) {
			SDL_RenderLine(renderer, view->dim.x, view->dim.y + i, view->dim.x + view->dim.w, view->dim.y + i);
		}
	}

	void draw_view_axes(const View *view) {
		if (view->x_axis.center >= view->dim.y && view->x_axis.center <= view->dim.y + view->dim.h) {
			thickLineRGBA(
				renderer,
				view->dim.x, view->x_axis.center,
				view->dim.x + view->dim.w, view->x_axis.center,
				2, CLR_NIGHT, SDL_ALPHA_OPAQUE
			);
		}
		if (view->y_axis.center >= view->dim.x && view->y_axis.center <= view->dim.x + view->dim.w) {
			thickLineRGBA(
				renderer,
				view->y_axis.center, view->dim.y,
				view->y_axis.center, view->dim.y + view->dim.h,
				2, CLR_NIGHT, SDL_ALPHA_OPAQUE
			);
		}
	}

	void draw_line_graph_stream(const View *view, const RingBuffer<float> *buf, double x0_base, double dx) {
		if (!view) return;
		const SDL_FRect r = view->dim;
		const size_t n = buf->size;
		if (n < 2) return;

		const double w_space = r.w / (double)view->x_axis.scale;
		size_t max_need = (size_t)w_space / dx + 3;  // just to be safe
		if (max_need > n) max_need = n;

		size_t start = n - max_need;
		double x_prev_space = x0_base + (double)start * dx;

		double x0 = view->x_space_to_screen(x_prev_space);
		double y0 = view->y_space_to_screen(buf->at(start));

		SDL_SetRenderDrawColor(renderer, CLR_PUCE, SDL_ALPHA_OPAQUE);
		for (size_t i = start + 1; i < n; ++i) {
			double x1_space = x0_base + (double)i * dx;
			double x1 = view->x_space_to_screen(x1_space);
			double y1 = view->y_space_to_screen(buf->at(i));

			double cx0 = x0, cy0 = y0, cx1 = x1, cy1 = y1;
			if (clip_to_rect(&cx0, &cy0, &cx1, &cy1, &r)) {
				SDL_RenderLine(renderer, (float)cx0, (float)cy0, (float)cx1, (float)cy1);
			}

			x0 = x1; y0 = y1;
		}
	}

	void text(const char *string, float x, float y) {
		TTF_Text *ttf_text = TTF_CreateText(text_engine, font, string, strlen(string));
		TTF_SetTextColor(ttf_text, CLR_NIGHT, SDL_ALPHA_OPAQUE);
		TTF_DrawRendererText(ttf_text, x, y);
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, CLR_PLATINUM, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}

	void present() {
		SDL_RenderPresent(renderer);
	}
};
