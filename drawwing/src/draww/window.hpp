#pragma once
#include <SDL3/SDL.h>
#include "axes.hpp"
#include "linalg.hpp"

#define CLR_BG 0, 0, 0
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

	void blit_axes(const CoordinateSystem &cs);
	void blit_grid(const CoordinateSystem &cs);
	void blit_cs_bg(const CoordinateSystem &cs);

	void draw_vector(const CoordinateSystem &cs, const Vector2 &vec);

	void draw_func(const CoordinateSystem &cs, float (fn)(float));

	void render_sphere(const CoordinateSystem &cs, const Sphere &sph, const Vector3 &light, const Vector3 &camera);
};
