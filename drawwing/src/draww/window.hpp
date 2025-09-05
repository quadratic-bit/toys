#pragma once
#include <SDL3/SDL.h>
#include "axes.hpp"
#include "vector.hpp"

#define CLR_BG 50, 50, 50
#define CLR_GRID 180, 180, 180
#define CLR_WHITE 255, 255, 255
#define CLR_BLACK 0, 0, 0

class DrawWindow {
public:
	SDL_Window *window;
	SDL_Renderer *renderer;

	DrawWindow(int width, int height);
	~DrawWindow();

	void clear();
	void present();

	void blit_coordinates(const CoordinateSystem &cs);
	void draw_vector(const CoordinateSystem &cs, Vector &vec);

	void draw_func(const CoordinateSystem &cs, float (fn)(float));
};
