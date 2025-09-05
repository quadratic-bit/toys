#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <cassert>
#include <stdexcept>

#include "axes.hpp"
#include "window.hpp"

static int pos_mod(int a, int b) {
	return (a % b + b) % b;
}

CoordinateSystem::CoordinateSystem(Axis x_ax, Axis y_ax, SDL_FRect dims) {
	x_axis = x_ax;
	y_axis = y_ax;
	dim = dims;
}

float CoordinateSystem::x_screen_to_space(float x_screen) const {
	return x_screen / (float)x_axis.scale;
}

float CoordinateSystem::y_screen_to_space(float y_screen) const {
	return y_screen / (float)y_axis.scale;
}

float CoordinateSystem::x_space_to_screen(float x_space) const {
	return y_axis.center + x_space * (float)x_axis.scale;
}

float CoordinateSystem::y_space_to_screen(float y_space) const {
	return x_axis.center + y_space * (float)y_axis.scale;
}

void DrawWindow::blit_coordinates(const CoordinateSystem &cs) {
	SDL_FRect surface;
	int grid_offset_x, grid_offset_y, i;

	// Surface
	SDL_SetRenderDrawColor(renderer, CLR_WHITE, SDL_ALPHA_OPAQUE);
	surface.x = cs.dim.x;
	surface.y = cs.dim.y;
	surface.w = cs.dim.w;
	surface.h = cs.dim.h;
	SDL_RenderFillRect(renderer, &surface);

	// Grid
	grid_offset_x = pos_mod(cs.y_axis.center - cs.dim.x, cs.x_axis.scale);
	grid_offset_y = pos_mod(cs.x_axis.center - cs.dim.y, cs.y_axis.scale);

	SDL_SetRenderDrawColor(renderer, CLR_GRID, SDL_ALPHA_OPAQUE);
	for (i = grid_offset_x; i <= cs.dim.w; i += cs.x_axis.scale) {
		SDL_RenderLine(renderer, cs.dim.x + i, cs.dim.y, cs.dim.x + i, cs.dim.y + cs.dim.h);
	}

	for (i = grid_offset_y; i <= cs.dim.h; i += cs.y_axis.scale) {
		SDL_RenderLine(renderer, cs.dim.x, cs.dim.y + i, cs.dim.x + cs.dim.w, cs.dim.y + i);
	}

	// Axes
	if (cs.x_axis.center >= cs.dim.y && cs.x_axis.center <= cs.dim.y + cs.dim.h) {
		thickLineRGBA(renderer, cs.dim.x, cs.x_axis.center, cs.dim.x + cs.dim.w, cs.x_axis.center, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
	}

	if (cs.y_axis.center >= cs.dim.x && cs.y_axis.center <= cs.dim.x + cs.dim.w) {
		thickLineRGBA(renderer, cs.y_axis.center, cs.dim.y, cs.y_axis.center, cs.dim.y + cs.dim.h, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
	}
}

void DrawWindow::draw_func(const CoordinateSystem &cs, float (fn)(float)) {
	assert(cs.dim.w >= 0);
	assert(cs.x_axis.scale > 0);
	assert(cs.y_axis.scale > 0);

	size_t n_pixels = (size_t)(cs.dim.w), i;
	SDL_FPoint *pixels = (SDL_FPoint *)malloc(n_pixels * sizeof(SDL_FPoint));

	if (pixels == NULL) {
		throw std::runtime_error("OOM");
	}

	float cs_x, value;
	for (i = 0; i < n_pixels; ++i) {
		cs_x = cs.x_screen_to_space(cs.y_axis.center - cs.dim.x + (float)i);
		value = fn(cs_x);
		pixels[i].x = cs.dim.x + i;
		pixels[i].y = cs.y_space_to_screen(value);
		if (pixels[i].y < cs.dim.y || pixels[i].y > cs.dim.y + cs.dim.h) {
			pixels[i].y = 0;
		}
	}
	SDL_SetRenderDrawColor(renderer, CLR_BLACK, SDL_ALPHA_OPAQUE);
	SDL_RenderPoints(renderer, pixels, n_pixels);
}
