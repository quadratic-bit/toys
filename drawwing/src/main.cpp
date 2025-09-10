#include "draww/linalg.hpp"
#include "draww/window.hpp"
#include "draww/axes.hpp"

static const int FPS = 60;
static const Uint64 FRAME_NS = SDL_NS_PER_SECOND / FPS;

static bool is_ev_close(const SDL_Event *event) {
	return event->type == SDL_EVENT_QUIT ||
		event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
}

int main() {
	SDL_Event ev;
	bool running = true;

	Axis x_axis_sphere = { 360, 30 };
	Axis y_axis_sphere = { 320, 30 };
	SDL_FRect cs_rect_sphere = { 0, 0, 640, 720 };
	CoordinateSystem *cs_sphere = new CoordinateSystem(
		x_axis_sphere,
		y_axis_sphere,
		cs_rect_sphere
	);
	Vector3 origin(0, 0, 0);
	Vector3 light(-10, 10, 20);
	Vector3 camera(0, 0, 10);
	Sphere sph(origin, 5);

	Axis x_axis_plane = { 360, 30 };
	Axis y_axis_plane = { 960, 30 };
	SDL_FRect cs_rect_plane = { 640, 0, 640, 720 };
	CoordinateSystem *cs_plane = new CoordinateSystem(
		x_axis_plane,
		y_axis_plane,
		cs_rect_plane
	);
	Vector2 sample(-1, -2);

	DrawWindow *window = new DrawWindow(1280, 720);

	Uint64 next_frame = SDL_GetTicksNS();

	while (running) {
		// FPS cap
		for (;;) {
			Uint64 now = SDL_GetTicksNS();
			if (now >= next_frame) break;

			Uint32 timeout_ms = (next_frame - now) / SDL_NS_PER_MS;

			if (timeout_ms == 0) break;

			if (SDL_WaitEventTimeout(&ev, timeout_ms)) {
				do {
					if (is_ev_close(&ev)) {
						running = false;
						break;
					}
				} while (running && SDL_PollEvent(&ev));
			}
			if (!running) break;
		}
		if (!running) break;

		while (SDL_PollEvent(&ev))
			if (is_ev_close(&ev))
				running = false;

		if (!running) break;

		// Rendering

		window->blit_bg(cs_sphere, CLR_VOID);
		window->render_sphere_with_ambient_diffusion_and_specular_light(
			cs_sphere,
			&sph,
			&light,
			&camera
		);
		light.rotate_xz(M_PI / 64);

		window->blit_bg(cs_plane, CLR_WHITE);
		window->blit_grid(cs_plane);
		window->blit_axes(cs_plane);
		window->blit_vector(cs_plane, &sample);
		sample.rotate(-M_PI / 64);

		window->present();

		// Tick management
		next_frame += FRAME_NS;
		Uint64 now = SDL_GetTicksNS();
		if (now > next_frame + FRAME_NS) next_frame = now;
	}

	delete window;
	delete cs_sphere;
	delete cs_plane;
	return 0;
}
