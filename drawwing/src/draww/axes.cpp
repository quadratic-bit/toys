#include <SDL3/SDL_system.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <stdexcept>

#include "axes.hpp"
#include "window.hpp"

static int pos_mod(int a, int b) {
	return (a % b + b) % b;
}

void DrawWindow::blit_axes(const CoordinateSystem * const cs) {
	if (cs->x_axis.center >= cs->dim.y && cs->x_axis.center <= cs->dim.y + cs->dim.h) {
		thickLineRGBA(renderer, cs->dim.x, cs->x_axis.center, cs->dim.x + cs->dim.w, cs->x_axis.center, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
	}

	if (cs->y_axis.center >= cs->dim.x && cs->y_axis.center <= cs->dim.x + cs->dim.w) {
		thickLineRGBA(renderer, cs->y_axis.center, cs->dim.y, cs->y_axis.center, cs->dim.y + cs->dim.h, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
	}
}

void DrawWindow::blit_grid(const CoordinateSystem * const cs) {
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

void DrawWindow::blit_bg(const CoordinateSystem * const cs, Uint8 r, Uint8 g, Uint8 b) {
	SDL_FRect surface;

	SDL_SetRenderDrawColor(renderer, r, g, b, SDL_ALPHA_OPAQUE);
	surface.x = cs->dim.x;
	surface.y = cs->dim.y;
	surface.w = cs->dim.w;
	surface.h = cs->dim.h;
	SDL_RenderFillRect(renderer, &surface);
}

void DrawWindow::draw_func(const CoordinateSystem * const cs, float (fn)(float)) {
	assert(cs->dim.w >= 0);
	assert(cs->x_axis.scale > 0);
	assert(cs->y_axis.scale > 0);

	size_t n_pixels = (size_t)(cs->dim.w), i;
	SDL_FPoint *pixels = (SDL_FPoint *)malloc(n_pixels * sizeof(SDL_FPoint));

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
	free(pixels);
}

static const int SPEC_POW = 30;

Uint8 quantize(Uint8 value, Uint8 steps);
Uint8 quantize(Uint8 value, Uint8 steps) {
	Uint8 scale = 255 / steps;
	return value / scale * scale;
}

void DrawWindow::render_sphere_with_ambient_diffusion_and_specular_light(
		const CoordinateSystem * const cs,
		const Sphere * const sph,
		const Vector3 * const light,
		const Vector3 * const camera
) {
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

			if (!sph->contains_2d(x, y)) {
				//pb->set_pixel_gray(pixels, pitch, sx, sy, quantize(RGB_VOID, 255));
				continue;
			}

			Vector3 point(x, y, sph->z_from_xy(x, y));
			Vector3 normal = sph->normal(point);

			Vector3 point_light = *light - point;
			Vector3 point_cam = *camera - point;

			// diffuse
			double cosalpha = !point_light ^ !normal;

			// NOTE: no need to normalize, as we need sign only
			double dir_cam_normal = point_cam ^ normal;
			double specular = 0;
			if (cosalpha > 0 && dir_cam_normal > 0) {
				Vector3 reflect = point_light.reflect(&normal);
				// specular
				double cosbeta = !point_cam ^ !reflect;

				if (cosbeta > 0) {
					specular = RGB_SPECULAR * std::pow(cosbeta, SPEC_POW);
				}
			}

			Uint8 lumin = std::min(
				RGB_AMBIENT +
				std::max(0.0, RGB_DIFFUSION * cosalpha) +
				specular,
				255.0
			);

			pb->set_pixel_gray(pixels, pitch, sx, sy, quantize(lumin, 255));
		}
	}

	pb->unlock();

	pb->draw();
}
