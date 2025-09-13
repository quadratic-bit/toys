#pragma once
#include <vector>

#include "axes.hpp"
#include "linalg.hpp"
#include "pixel_buffer.hpp"

#define RGB_BLACK 0
#define RGB_WHITE 255

#define RGB_LIGHT_GRAY 180
#define RGB_VOID 21

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

struct RenderContext {
	Vector3 point;
	Vector3 normal;
	const Sphere *sph;
	const Vector3 *camera;
};

struct CamBasis {
	Vector3 pos;
	Vector3 fwd;   // normalized
	Vector3 right; // normalized
	Vector3 up;    // normalized
	double  focal; // focal length in world units, |camera->dir|
};

struct Camera {
	Vector3 pos;
	Vector3 dir;
};

class Scene {
	CoordinateSystem *cs;
	SDL_Renderer *renderer;
	PixelBuffer *pb;

	bool project_point_perspective(const Vector3 &p, const Camera *camera, SDL_FPoint *out) const;

	double shadow_factor_to_light(const Vector3 &light, const Vector3 &point, const Sphere *exclude) const;
	double calculate_light(const Vector3 &light, const RenderContext &ctx) const;
	double accum_lights(const RenderContext &ctx) const;

	const Sphere *sphere_intersect(double *hit, const CamBasis &cb, const Vector3 &ray_dir) const;

	bool is_occluded(const Vector3 &A, const Vector3 &B) const;
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

	void blit_axes_3d(const Camera *camera, double len_units);

	void blit_light_sources(const Camera *camera, int radius_px, bool occlusion_test);
	void render_with_ambient_diffusion_and_specular_light(const Camera *camera);
};
