#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <limits>
#include <stdexcept>

#include "scene.hpp"

static inline int pos_mod(int a, int b) {
	return (a % b + b) % b;
}

static inline Uint8 quantize(Uint8 value, Uint8 steps) {
	Uint8 scale = 255 / steps;
	return value / scale * scale;
}

static inline double clamp01(double x) {
	return x < 0 ? 0 : (x > 1 ? 1 : x);
}

static inline double atan_penumbra(double dist, double radius, double width) {
	static const double SHADOW_ROUGH = 8.0;  // arctg roughness

	if (dist <= radius) return 0.0;
	// f(l0)=0, f(l0+w)=1
	const double num = std::atan(SHADOW_ROUGH * (dist - radius));
	const double den = std::atan(SHADOW_ROUGH * width) + 1e-12;
	return clamp01(num / den);
}

void Scene::blit_vector(Vector2 vec) {
	static const double ARROW_HEAD_SIZE = 0.5;

	Vector2 norm = !vec;
	Vector2 right_wing = (norm.perp_right() - norm) * ARROW_HEAD_SIZE + vec;
	Vector2 left_wing  = (norm.perp_left()  - norm) * ARROW_HEAD_SIZE + vec;

	cs->vector2_space_to_screen(&vec);
	cs->vector2_space_to_screen(&right_wing);
	cs->vector2_space_to_screen(&left_wing);

	// Vector
	thickLineRGBA(
		renderer,
		cs->y_axis.center, cs->x_axis.center,
		vec.x, vec.y,
		3, CLR_BLACK, SDL_ALPHA_OPAQUE
	);

	// Head
	thickLineRGBA(
		renderer,
		right_wing.x, right_wing.y, vec.x, vec.y,
		3, CLR_BLACK, SDL_ALPHA_OPAQUE
	);
	thickLineRGBA(
		renderer,
		left_wing.x, left_wing.y, vec.x, vec.y,
		3, CLR_BLACK, SDL_ALPHA_OPAQUE
	);
}

void Scene::blit_axes() {
	if (cs->x_axis.center >= cs->dim.y && cs->x_axis.center <= cs->dim.y + cs->dim.h) {
		thickLineRGBA(
			renderer,
			cs->dim.x, cs->x_axis.center,
			cs->dim.x + cs->dim.w, cs->x_axis.center,
			3, CLR_BLACK, SDL_ALPHA_OPAQUE
		);
	}

	if (cs->y_axis.center >= cs->dim.x && cs->y_axis.center <= cs->dim.x + cs->dim.w) {
		thickLineRGBA(
			renderer,
			cs->y_axis.center, cs->dim.y,
			cs->y_axis.center, cs->dim.y + cs->dim.h,
			3, CLR_BLACK, SDL_ALPHA_OPAQUE
		);
	}
}

void Scene::blit_grid() {
	int grid_offset_x, grid_offset_y, i;

	grid_offset_x = pos_mod(cs->y_axis.center - cs->dim.x, cs->x_axis.scale);
	grid_offset_y = pos_mod(cs->x_axis.center - cs->dim.y, cs->y_axis.scale);

	SDL_SetRenderDrawColor(renderer, CLR_GRID, SDL_ALPHA_OPAQUE);
	for (i = grid_offset_x; i <= cs->dim.w; i += cs->x_axis.scale) {
		SDL_RenderLine(renderer, cs->dim.x + i, cs->dim.y, cs->dim.x + i, cs->dim.y + cs->dim.h);
	}

	for (i = grid_offset_y; i <= cs->dim.h; i += cs->y_axis.scale) {
		SDL_RenderLine(renderer, cs->dim.x, cs->dim.y + i, cs->dim.x + cs->dim.w, cs->dim.y + i);
	}
}

void Scene::blit_bg(Uint8 r, Uint8 g, Uint8 b) {
	SDL_FRect surface;

	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	surface.x = cs->dim.x;
	surface.y = cs->dim.y;
	surface.w = cs->dim.w;
	surface.h = cs->dim.h;
	SDL_RenderFillRect(renderer, &surface);
}

void Scene::draw_func(double (fn)(double)) {
	assert(cs->dim.w >= 0);
	assert(cs->x_axis.scale > 0);
	assert(cs->y_axis.scale > 0);

	size_t n_pixels = (size_t)(cs->dim.w), i;
	SDL_FPoint *pixels = new SDL_FPoint[n_pixels];

	if (pixels == NULL) {
		throw std::runtime_error("OOM");
	}

	float cs_x, value;
	for (i = 0; i < n_pixels; ++i) {
		cs_x = cs->x_screen_to_space(cs->dim.x + (float)i);
		value = fn(cs_x);
		pixels[i].x = cs->dim.x + i;
		pixels[i].y = cs->y_space_to_screen(value);
		if (pixels[i].y < cs->dim.y || pixels[i].y > cs->dim.y + cs->dim.h) {
			pixels[i].y = 0;
		}
	}
	SDL_SetRenderDrawColor(renderer, CLR_BLACK, SDL_ALPHA_OPAQUE);
	SDL_RenderPoints(renderer, pixels, n_pixels);
	delete[] pixels;
}

double Scene::shadow_factor_to_light(const Vector3& light, const Vector3& point, const Sphere* exclude) const {
	static const double EARLY_EXIT = 1e-3;
	static const double REL_W = 0.30;  // penumbra width phrased as a radius fraction

	Vector3 segment = light - point;
	double len = segment.length();
	Vector3 dir = !segment;

	double shadow = 1.0;

	for (size_t i = 0; i < spheres.size(); ++i) {
		const Sphere* s = &spheres[i];
		if (s == exclude) continue;

		// project sphere center onto (point + t*d) ray
		double t = (s->pos - point) ^ dir;
		if (t <= 0.0 || t >= len) continue;

		Vector3 closest = point + dir * t;
		Vector3 diff = s->pos - closest;
		double delta = diff.length();

		// hard shadow
		if (delta < s->radius) return 0.0;

		// penumbra
		const double w = REL_W * s->radius;
		double fac = atan_penumbra(delta, s->radius, w);
		shadow *= fac;

		if (shadow < EARLY_EXIT) return 0.0;
	}

	return clamp01(shadow);
}

const Sphere *Scene::nearest_sphere(Vector3 *vec) const {
	const Sphere *sph = NULL;
	vec->z = -std::numeric_limits<double>::infinity();

	for (size_t sph_i = 0; sph_i < spheres.size(); ++sph_i) {
		if (!spheres[sph_i].contains_2d(vec->x, vec->y)) continue;
		double z = spheres[sph_i].z_from_xy(vec->x, vec->y);
		if (z > vec->z) {
			vec->z = z;
			sph = &spheres[sph_i];
		}
	}
	return sph;
}

double Scene::calculate_light(const Vector3 &light, const RenderContext &ctx) const {
	static const int SPEC_POW = 30;

	double shadow = this->shadow_factor_to_light(light, ctx.point, ctx.sph);
	if (shadow <= 0.0) return 0;

	Vector3 point_light = light       - ctx.point;
	Vector3 point_cam   = *ctx.camera - ctx.point;

	// diffuse
	double cosalpha = !point_light ^ !ctx.normal;

	double specular = 0.0;
	// NOTE: no need to normalize, as we need sign only
	double dir_cam_normal = point_cam ^ ctx.normal;
	if (cosalpha > 0.0 && dir_cam_normal > 0.0) {
		Vector3 reflect = point_light.reflect(&ctx.normal);
		// specular
		double cosbeta = !point_cam ^ !reflect;

		if (cosbeta > 0.0) {
			specular = RGB_SPECULAR * std::pow(cosbeta, SPEC_POW);
		}
	}

	return shadow * (std::max(0.0, RGB_DIFFUSION * cosalpha) + specular);
}

double Scene::accum_lights(const RenderContext &ctx) const {
	double lumin = RGB_AMBIENT;
	for (size_t light_i = 0; light_i < light_sources.size(); ++light_i) {
		const Vector3 *light = &light_sources[light_i];
		double added_lumin = calculate_light(*light, ctx);
		lumin += added_lumin;
	}
	return std::min(lumin, 255.0);
}

void Scene::render_with_ambient_diffusion_and_specular_light(const Vector3 * const camera) {
	SDL_Rect lock = { (int)(cs->dim.x), (int)(cs->dim.y), (int)(cs->dim.w), (int)(cs->dim.h) };

	void *pixels = NULL;
	int pitch = 0;
	if (!pb->lock(&lock, &pixels, &pitch)) {
		SDL_Log("LockTexture failed: %s", SDL_GetError());
		return;
	}

	for (int sy = 0; sy < lock.h; ++sy) {
		int h = lock.y + sy;
		double y = cs->y_screen_to_space(h);

		for (int sx = 0; sx < lock.w; ++sx) {
			int w = lock.x + sx;
			double x = cs->x_screen_to_space(w);

			Uint8 lumin = RGB_VOID;

			Vector3 point(x, y, 0);
			const Sphere *sph = nearest_sphere(&point);

			if (sph == NULL) {
				pb->set_pixel_gray(pixels, pitch, sx, sy, quantize(lumin, 255));
				continue;
			}

			Vector3 normal = sph->normal(point);
			RenderContext ctx = { point, normal, sph, camera };

			lumin = accum_lights(ctx);

			pb->set_pixel_gray(pixels, pitch, sx, sy, quantize(lumin, 255));
		}
	}

	pb->unlock();

	pb->draw();
}
