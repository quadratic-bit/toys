#pragma once
#include <SDL3/SDL.h>

struct Axis {
	float center;  // absolute
	int scale;  // pixels per unit
};

class CoordinateSystem {
public:
	Axis x_axis;
	Axis y_axis;
	SDL_FRect dim;

	CoordinateSystem(Axis x_ax, Axis y_ax, SDL_FRect dims);

	float x_screen_to_space(float x_screen) const;
	float y_screen_to_space(float y_screen) const;
	float x_space_to_screen(float x_space) const;
	float y_space_to_screen(float y_space) const;
};
