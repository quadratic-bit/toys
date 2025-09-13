#pragma once
#include <SDL3/SDL.h>

#include "linalg.hpp"

struct Axis {
	double center;  // absolute
	double scale;  // pixels per unit
};

class CoordinateSystem {
public:
	Axis x_axis;
	Axis y_axis;
	Axis z_axis;
	SDL_FRect dim;

	CoordinateSystem(Axis x_ax, Axis y_ax, Axis z_ax, SDL_FRect dims)
		: x_axis(x_ax), y_axis(y_ax), z_axis(z_ax), dim(dims) {}

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

	void vector2_space_to_screen(Vector2 *vec) const {
		vec->x = x_space_to_screen(vec->x);
		vec->y = y_space_to_screen(vec->y);
	}

	float principal_x_px() const {
		return (float)y_axis.center;
	}

	float principal_y_px() const {
		return (float)x_axis.center;
	}

	double units_per_px_x() const {
		return 1.0 / x_axis.scale;
	}

	double units_per_px_y() const {
		return 1.0 / y_axis.scale;
	}
};
