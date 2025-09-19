#pragma once
#include <SDL3/SDL.h>

#include "vectors.hpp"

struct Axis {
	double center;  // absolute
	double scale;   // pixels per unit
};

static inline int pos_mod(int a, int b) {
	return (a % b + b) % b;
}

class View {
public:
	Axis x_axis;
	Axis y_axis;
	SDL_FRect dim;

	View(Axis x_ax, Axis y_ax, SDL_FRect dims)
		: x_axis(x_ax), y_axis(y_ax), dim(dims) {}

	void pin_to_right(double x_latest) {
		y_axis.center = dim.x + dim.w - x_latest * (double)x_axis.scale;
	}

	double x_screen_to_space(double x_screen) const {
		return (x_screen - y_axis.center) / (double)x_axis.scale;
	}

	double y_screen_to_space(double y_screen) const {
		return (x_axis.center - y_screen) / (double)y_axis.scale;
	}

	double x_space_to_screen(double x_space) const {
		return y_axis.center + x_space * (double)x_axis.scale;
	}

	double y_space_to_screen(double y_space) const {
		return x_axis.center - y_space * (double)y_axis.scale;
	}

	void space_to_screen(Vector2 *vec) const {
		vec->x = x_space_to_screen(vec->x);
		vec->y = y_space_to_screen(vec->y);
	}

	void screen_to_space(Vector2 *vec) const {
		vec->x = x_screen_to_space(vec->x);
		vec->y = y_screen_to_space(vec->y);
	}
};

// Cohen–Sutherland algorithm (clipping lines)

enum {
	CS_INSIDE = 0,
	CS_LEFT   = 1 << 0,
	CS_RIGHT  = 1 << 1,
	CS_BOTTOM = 1 << 2,
	CS_TOP    = 1 << 3
};

static inline int cs_outcode(double x, double y, const SDL_FRect *r) {
	int code = CS_INSIDE;

	if (x < r->x) code |= CS_LEFT;
	else if (x > r->x + r->w) code |= CS_RIGHT;

	if (y < r->y) code |= CS_TOP;
	else if (y > r->y + r->h) code |= CS_BOTTOM;

	return code;
}

// return true if something remains
static bool clip_to_rect(double *x0, double *y0, double *x1, double *y1, const SDL_FRect *r) {
	int out0 = cs_outcode(*x0, *y0, r);
	int out1 = cs_outcode(*x1, *y1, r);

	while (1) {
		if (!(out0 | out1)) return true;  // trivially accept
		if (out0 & out1) return false;    // trivially reject

		// pick an endpoint outside the rect
		int out = out0 ? out0 : out1;
		double x, y;

		if (out & CS_TOP) { // y < top
			x = *x0 + (*x1 - *x0) * (r->y - *y0) / (*y1 - *y0);
			y = r->y;
		} else if (out & CS_BOTTOM) { // y > bottom
			x = *x0 + (*x1 - *x0) * ((r->y + r->h) - *y0) / (*y1 - *y0);
			y = r->y + r->h;
		} else if (out & CS_RIGHT) { // x > right
			y = *y0 + (*y1 - *y0) * ((r->x + r->w) - *x0) / (*x1 - *x0);
			x = r->x + r->w;
		} else { // CS_LEFT: x < left
			y = *y0 + (*y1 - *y0) * (r->x - *x0) / (*x1 - *x0);
			x = r->x;
		}

		if (out == out0) {
			*x0 = x;
			*y0 = y;
			out0 = cs_outcode(*x0, *y0, r);
		} else {
			*x1 = x;
			*y1 = y;
			out1 = cs_outcode(*x1, *y1, r);
		}
	}
}
