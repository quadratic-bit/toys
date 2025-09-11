#pragma once

#include "axes.hpp"
#include "linalg.hpp"
#include "pixel_buffer.hpp"
#include <vector>

#define RGB_BLACK 0
#define RGB_WHITE 255

#define RGB_LIGHT_GRAY 180
#define RGB_VOID 18

#define RGB_AMBIENT 26
#define RGB_DIFFUSION 147
#define RGB_SPECULAR 100

#define CLR_MONO(v) v, v, v

#define CLR_BLACK CLR_MONO(RGB_BLACK)
#define CLR_WHITE CLR_MONO(RGB_WHITE)
#define CLR_GRID CLR_MONO(RGB_LIGHT_GRAY)

#define CLR_BG CLR_BLACK

#define CLR_AMBIENT CLR_MONO(RGB_AMBIENT)
#define CLR_VOID CLR_MONO(RGB_VOID)

class Scene {
	CoordinateSystem *cs;
	SDL_Renderer *renderer;
	PixelBuffer *pb;

	bool any_sphere_intersects(const Vector3 &light, const Vector3 &point, size_t exclude);

public:
	std::vector<Sphere> spheres;
	std::vector<Vector3> light_sources;

	Scene(
		CoordinateSystem *coords,
		SDL_Renderer *rend,
		PixelBuffer *buf
	     ) : cs(coords), renderer(rend), pb(buf) {
		spheres = std::vector<Sphere>();
		light_sources = std::vector<Vector3>();
	};

	void blit_axes();
	void blit_grid();
	void blit_bg(Uint8 r, Uint8 g, Uint8 b);
	void blit_vector(Vector2 vec);

	void draw_func(double (fn)(double));

	void render_with_ambient_diffusion_and_specular_light(const Vector3 *camera);
};
