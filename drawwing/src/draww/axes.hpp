#pragma once
#include <SDL3/SDL.h>

#include "linalg.hpp"

struct Axis {
	float center;  // absolute
	int scale;  // pixels per unit
};

class CoordinateSystem {
public:
	Axis x_axis;
	Axis y_axis;
	SDL_FRect dim;

	CoordinateSystem(Axis x_ax, Axis y_ax, SDL_FRect dims)
		: x_axis(x_ax), y_axis(y_ax), dim(dims) {}

	float x_screen_to_space(float x_screen) const {
		return (x_screen - y_axis.center) / (float)x_axis.scale;
	}

	float y_screen_to_space(float y_screen) const {
		return (x_axis.center - y_screen) / (float)y_axis.scale;
	}

	float x_space_to_screen(float x_space) const {
		return y_axis.center + x_space * (float)x_axis.scale;
	}

	float y_space_to_screen(float y_space) const {
		return x_axis.center - y_space * (float)y_axis.scale;
	}

	void vector2_space_to_screen(Vector2 *vec) const {
		vec->x = x_space_to_screen(vec->x);
		vec->y = y_space_to_screen(vec->y);
	}
};
