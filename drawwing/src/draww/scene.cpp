#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <limits>
#include <stdexcept>

#include "scene.hpp"

// Floor
static const double FLOOR_Y = -20.0;  // XZ plane

static inline double checker_albedo(double x, double z) {
	static const double CHECK_SIZE = 2.5;  // in world units
	static const double CHECK_ALBEDO_LIGHT = 0.85;
	static const double CHECK_ALBEDO_DARK  = 0.30;

	int ix = std::floor(x / CHECK_SIZE);
	int iz = std::floor(z / CHECK_SIZE);
	return ((ix + iz) & 1) ? CHECK_ALBEDO_DARK : CHECK_ALBEDO_LIGHT;
}
static inline double darken_by_distance(double dist) {
	static const double DIST_DARKEN_K = 0.010; // in world units

	double a = std::exp(-DIST_DARKEN_K * std::max(0.0, dist));
	return a < 0.0 ? 0.0 : (a > 1.0 ? 1.0 : a);
}

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

static inline CamBasis make_cam_basis(const Camera *camera) {
	CamBasis cb;
	cb.pos   = camera->pos;
	cb.focal = camera->dir.length();
	cb.fwd   = !camera->dir;

	Vector3 world_up(0.0, 1.0, 0.0);

	if (std::abs(cb.fwd ^ world_up) > 0.999) world_up = Vector3(0.0, 0.0, 1.0);

	cb.right = !(cb.fwd   % world_up);
	cb.up    =  (cb.right % cb.fwd);
	return cb;
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
bool Scene::project_point_perspective(const Vector3 &p, const Camera *camera, SDL_FPoint *out) const {
	CamBasis cb = make_cam_basis(camera);

	// camera-space
	Vector3 v = p - cb.pos;
	double zc = v ^ cb.fwd;
	if (zc <= 1e-6) return false;  // behind or too close

	double xc = v ^ cb.right;
	double yc = v ^ cb.up;

	// image plane at distance f along +fwd
	double u  = cb.focal * (xc / zc);   // world-X on image plane
	double v2 = cb.focal * (yc / zc);   // world-Y on image plane

	out->x = cs->principal_x_px() + (float)(u  * cs->x_axis.scale);
	out->y = cs->principal_y_px() - (float)(v2 * cs->y_axis.scale);
	return (out->x >= cs->dim.x && out->x <= cs->dim.x + cs->dim.w &&
	        out->y >= cs->dim.y && out->y <= cs->dim.y + cs->dim.h);
}

void Scene::blit_axes_3d(const Camera* camera, double len_units = 6.0) {
	SDL_FPoint o2d, x2d, y2d, z2d;
	Vector3 O = Vector3();
	if (!project_point_perspective(O, camera, &o2d)) return;

	Vector3 X(len_units, 0, 0), Y(0, len_units, 0), Z(0, 0, len_units);

	bool vx = project_point_perspective(X, camera, &x2d);
	bool vy = project_point_perspective(Y, camera, &y2d);
	bool vz = project_point_perspective(Z, camera, &z2d);

	if (vx) thickLineRGBA(renderer, o2d.x, o2d.y, x2d.x, x2d.y, 3, 255, 0, 0, SDL_ALPHA_OPAQUE);
	if (vy) thickLineRGBA(renderer, o2d.x, o2d.y, y2d.x, y2d.y, 3, 0, 255, 0, SDL_ALPHA_OPAQUE);
	if (vz) thickLineRGBA(renderer, o2d.x, o2d.y, z2d.x, z2d.y, 3, 0, 0, 255, SDL_ALPHA_OPAQUE);
}

const Sphere *Scene::sphere_intersect(double *hit, const CamBasis &cb, const Vector3 &ray_dir) const {
	const Sphere *sph = NULL;
	*hit = std::numeric_limits<double>::infinity();

	for (size_t i = 0; i < spheres.size(); ++i) {
		const Sphere& s = spheres[i];
		Vector3 oc = cb.pos - s.pos;
		double b = oc ^ ray_dir;
		double c = (oc ^ oc) - s.radius * s.radius;
		double disc = b * b - c;
		if (disc < 0.0) continue;

		double t = -b - std::sqrt(disc);
		if (t <= 1e-6) t = -b + std::sqrt(disc);
		if (t > 1e-6 && t < *hit) {
			*hit = t;
			sph = &s;
		}
	}
	return sph;
}

bool Scene::is_occluded(const Vector3 &A, const Vector3 &B) const {
	static const double EPS_RAY = 1e-6;

	Vector3 d = B - A;
	double tMax = d.length();
	if (tMax <= EPS_RAY) return false;
	Vector3 dir = !d;

	for (size_t i = 0; i < spheres.size(); ++i) {
		const Sphere &s = spheres[i];
		Vector3 oc = A - s.pos;
		double b = oc ^ dir;
		double c = (oc ^ oc) - s.radius * s.radius;
		double disc = b * b - c;
		if (disc < 0.0) continue;

		double sqrt_disc = std::sqrt(disc);
		double t = -b - sqrt_disc;
		if (t <= EPS_RAY) t = -b + sqrt_disc;
		if (t > EPS_RAY && t < tMax - EPS_RAY) return true;
	}

	if (std::abs(dir.y) > EPS_RAY) {
		double t = (FLOOR_Y - A.y) / dir.y;
		if (t > EPS_RAY && t < tMax - EPS_RAY) return true;
	}

	return false;
}

void Scene::blit_light_sources(const Camera *camera, int radius_px, bool occlusion_test) {
	for (size_t i = 0; i < light_sources.size(); ++i) {
		const Vector3 *L = &light_sources[i];
		if (occlusion_test && is_occluded(camera->pos, *L)) continue;

		SDL_FPoint p2d;
		if (!project_point_perspective(*L, camera, &p2d)) continue;

		if (p2d.x < cs->dim.x || p2d.x > cs->dim.x + cs->dim.w ||
				p2d.y < cs->dim.y || p2d.y > cs->dim.y + cs->dim.h)
			continue;

		filledCircleRGBA(
			renderer,
			lround(p2d.x),
			lround(p2d.y),
			radius_px,
			255, 255, 255, SDL_ALPHA_OPAQUE
		);
	}
}

void Scene::render_with_ambient_diffusion_and_specular_light(const Camera * const camera) {
	SDL_Rect lock = { (int)(cs->dim.x), (int)(cs->dim.y), (int)(cs->dim.w), (int)(cs->dim.h) };

	void *pixels = NULL;
	int pitch = 0;
	if (!pb->lock(&lock, &pixels, &pitch)) {
		SDL_Log("LockTexture failed: %s", SDL_GetError());
		return;
	}

	CamBasis cb = make_cam_basis(camera);
	if (cb.focal <= 0.0) {
		pb->unlock();
		pb->draw();
		return;
	}

	const double ux = cs->units_per_px_x();
	const double uy = cs->units_per_px_y();
	const float  cx = cs->principal_x_px();
	const float  cy = cs->principal_y_px();

	const Vector3 img_center = cb.pos + cb.fwd * cb.focal;

	for (int sy = 0; sy < lock.h; ++sy) {
		const double py = lock.y + sy + 0.5;
		const double dy_units = -(py - cy) * uy;

		for (int sx = 0; sx < lock.w; ++sx) {
			const double px = lock.x + sx + 0.5;
			const double dx_units = (px - cx) * ux;

			// point on image plane in world
			Vector3 P = img_center + cb.right * dx_units + cb.up * dy_units;

			Vector3 ray_dir = !(P - cb.pos);
			double hitT;
			const Sphere *hit_sph = sphere_intersect(&hitT, cb, ray_dir);

			bool hit_plane = false;
			if (std::abs(ray_dir.y) > 1e-3) {
				double t = (FLOOR_Y - cb.pos.y) / ray_dir.y;
				if (t > 1e-3 && t < hitT) {
					hit_plane = true;
					hit_sph = NULL;
					hitT = t;
				}
			}

			Uint8 lumin8 = RGB_VOID;

			if (hitT == std::numeric_limits<double>::infinity()) {
				pb->set_pixel_gray(pixels, pitch, sx, sy, lumin8);
				continue;
			}

			Vector3 hit_point  = cb.pos + ray_dir * hitT;

			if (hit_plane) {
				const Vector3 hit_normal(0.0, 1.0, 0.0);

				RenderContext ctx = { hit_point, hit_normal, NULL, &camera->pos };
				double lumin  = accum_lights(ctx);
				double albedo = checker_albedo(hit_point.x, hit_point.z);
				double atten  = darken_by_distance(hitT);

				double finalL = std::min(255.0, lumin * albedo * atten);
				lumin8 = quantize((Uint8)lround(finalL), 255);
			} else {
				const Vector3 hit_normal = !(hit_point - hit_sph->pos);

				RenderContext ctx = { hit_point, hit_normal, hit_sph, &camera->pos };
				double lumin = accum_lights(ctx);
				lumin8 = quantize((Uint8)round(lumin), 255);
			}

			pb->set_pixel_gray(pixels, pitch, sx, sy, lumin8);
		}
	}

	pb->unlock();
	pb->draw();
}
