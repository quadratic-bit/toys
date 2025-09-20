#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cstdio>
#include <stdexcept>

#include "pixel_buffer.hpp"
#include "reactor.hpp"

enum TextAlign { TA_LEFT, TA_CENTER, TA_RIGHT };

class AReactorWindow {
	SDL_Window *window;
	TTF_TextEngine *text_engine;
	TTF_Font *font;
	PixelBuffer *pb;

	Reactor *reactor;

	void draw_bounding_line(Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, unsigned thick) {
		thickLineRGBA(renderer, x1, y1, x2, y2, thick, CLR_NIGHT, SDL_ALPHA_OPAQUE);
	}

public:
	SDL_Renderer *renderer;

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

	void text(const char *string, float x, float y) {
		TTF_Text *tt = TTF_CreateText(text_engine, font, string, strlen(string));
		if (!tt) return;

		TTF_SetTextColor(tt, CLR_NIGHT, SDL_ALPHA_OPAQUE);
		TTF_DrawRendererText(tt, x, y);
		TTF_DestroyText(tt);
	}

	void text_aligned(const char *string, float x, float y, TextAlign align = TA_LEFT, bool vcenter = true) {
		TTF_Text *tt = TTF_CreateText(text_engine, font, string, strlen(string));
		if (!tt) return;

		int w = 0, h = 0;
		TTF_GetTextSize(tt, &w, &h);

		if (align == TA_CENTER) x -= w * 0.5;
		else if (align == TA_RIGHT) x -= (float)w;
		if (vcenter) y -= h * 0.5;

		TTF_SetTextColor(tt, CLR_NIGHT, SDL_ALPHA_OPAQUE);
		TTF_DrawRendererText(tt, x, y);
		TTF_DestroyText(tt);
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, CLR_PLATINUM, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}

	void present() {
		SDL_RenderPresent(renderer);
	}
};
