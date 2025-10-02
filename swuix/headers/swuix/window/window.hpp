#pragma once
#include <SDL3/SDL.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <cstdio>
#include <stdexcept>
#include <vector>

#include <swuix/window/common.hpp>
#include <swuix/window/pixel_buffer.hpp>
#include <swuix/common.hpp>

enum TextAlign { TA_LEFT, TA_CENTER, TA_RIGHT };

#define DEBUG_THICKNESS   2
#define DEBUG_ALPHA       (uint8_t)(255 * 0.5f)
#define DEBUG_PATTERN_GAP 12.0f

static inline int round_int(float v) {
	return (int)floorf(v + 0.5f);
}

class Window {
	SDL_Window *window;
	TTF_TextEngine *text_engine;
	TTF_Font *font;
	PixelBuffer *pb;

public:
	SDL_Renderer *renderer;

	Window(int width, int height) {
		SDL_SetAppMetadata("UI-core", "0.1", "com.toy.uicore");

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		if (!SDL_CreateWindowAndRenderer(
				"ui-core", width, height, SDL_WindowFlags(0),
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

	~Window() {
		delete pb;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	// ================ PRIMITIVES ================

	void draw_line_rgb(int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned thick, uint8_t r, uint8_t g, uint8_t b) {
		thickLineRGBA(renderer, x1, y1, x2, y2, thick, r, g, b, SDL_ALPHA_OPAQUE);
	}

	void draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, unsigned thick) {
		draw_line_rgb(x1, y1, x2, y2, thick, CLR_NIGHT);
	}

	void draw_circle_rgb(float x, float y, float rad, uint8_t r, uint8_t g, uint8_t b) {
		SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
		circleRGBA(renderer, x, y, rad, r, g, b, SDL_ALPHA_OPAQUE);
	}

	void draw_filled_circle_rgb(float x, float y, float rad, uint8_t r, uint8_t g, uint8_t b) {
		SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
		filledCircleRGBA(renderer, x, y, rad, r, g, b, SDL_ALPHA_OPAQUE);
	}

	void draw_filled_rect_rgb(const FRect &rect, uint8_t r, uint8_t g, uint8_t b) {
		SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(renderer, &rect);
	}

	// =================== TEXT ===================

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

	void outline(const FRect box, int off_x, int off_y, unsigned thick = 2) {
		int16_t x = box.x + off_x, y = box.y + off_y;
		int16_t w = box.w, h = box.h;
		draw_line(x, y, x + w, y, thick);
		draw_line(x, y, x, y + h, thick);
		draw_line(x + w, y, x + w, y + h, thick);
		draw_line(x, y + h, x + w, y + h, thick);
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, CLR_PLATINUM, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}

	void clear_rect(FRect box, int off_x, int off_y, uint8_t r, uint8_t g, uint8_t b) {
		box.x += off_x;
		box.y += off_y;
		SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
		SDL_RenderFillRect(renderer, &box);
	}

	void debug_outline(const FRect box, int off_x, int off_y) {
		if (box.w <= 0.0f || box.h <= 0.0f) return;

		const float x0 = box.x + off_x;
		const float y0 = box.y + off_y;
		const float x1 = box.x + off_x + box.w;
		const float y1 = box.y + off_y + box.h;

		int lx0 = round_int(x0), ly0 = round_int(y0);
		int lx1 = round_int(x1), ly1 = round_int(y1);

		thickLineRGBA(renderer, lx0, ly0, lx1, ly0, DEBUG_THICKNESS, CLR_DEBUG, DEBUG_ALPHA);
		thickLineRGBA(renderer, lx0, ly1, lx1, ly1, DEBUG_THICKNESS, CLR_DEBUG, DEBUG_ALPHA);
		thickLineRGBA(renderer, lx0, ly0, lx0, ly1, DEBUG_THICKNESS, CLR_DEBUG, DEBUG_ALPHA);
		thickLineRGBA(renderer, lx1, ly0, lx1, ly1, DEBUG_THICKNESS, CLR_DEBUG, DEBUG_ALPHA);

		const float gap = DEBUG_PATTERN_GAP;
		if (gap <= 0.0f) return;
		const float dk = gap * sqrtf(2.0f);

		const float kmin = x0 + y0;
		const float kmax = x1 + y1;

		float kstart = floorf(kmin / dk) * dk;

		float yi, xi;
		for (float k = kstart; k <= kmax; k += dk) {
			std::vector<SDL_FPoint> pts;
			pts.reserve(4);

			yi = k - x0;
			if (yi >= y0 - 1e-4f && yi <= y1 + 1e-4f) {
				SDL_FPoint p = { x0, yi };
				pts.push_back(p);
			}

			yi = k - x1;
			if (yi >= y0 - 1e-4f && yi <= y1 + 1e-4f) {
				SDL_FPoint p = { x1, yi };
				pts.push_back(p);
			}

			xi = k - y0;
			if (xi >= x0 - 1e-4f && xi <= x1 + 1e-4f) {
				SDL_FPoint p = { xi, y0 };
				pts.push_back(p);
			}

			xi = k - y1;
			if (xi >= x0 - 1e-4f && xi <= x1 + 1e-4f) {
				SDL_FPoint p = { xi, y1 };
				pts.push_back(p);
			}

			if (pts.size() < 2) continue;

			// find two unique points
			SDL_FPoint a = pts[0];
			SDL_FPoint b;
			bool found = false;
			for (size_t i = 1; i < pts.size(); ++i) {
				if (fabsf(pts[i].x - a.x) > 1e-4f || fabsf(pts[i].y - a.y) > 1e-4f) {
					b = pts[i];
					found = true;
					break;
				}
			}
			if (!found) continue;

			int ax = round_int(a.x);
			int ay = round_int(a.y);
			int bx = round_int(b.x);
			int by = round_int(b.y);

			thickLineRGBA(renderer, ax, ay, bx, by, DEBUG_THICKNESS, CLR_DEBUG, DEBUG_ALPHA);
		}
	}

	void present() {
		SDL_RenderPresent(renderer);
	}
};
