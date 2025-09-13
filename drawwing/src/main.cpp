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
	Axis z_axis_sphere = {   0, 30 };

	SDL_FRect cs_rect_sphere = { 0, 0, 640, 720 };
	CoordinateSystem *cs_sphere = new CoordinateSystem(
		x_axis_sphere,
		y_axis_sphere,
		z_axis_sphere,
		cs_rect_sphere
	);

	Camera camera = { Vector3(0, 5, 15), Vector3(0, -5, -10) };

	Vector3 light(-19, 9, 20);

	Vector3 origin1(-5, -5, -14);
	Vector3 origin2( 5,  5,   0);
	Vector3 origin3(-5, -5,   0);
	Sphere sph1(origin1, 7);
	Sphere sph2(origin2, 2);
	Sphere sph3(origin3, 4);

	Axis x_axis_plane = { 360, 30 };
	Axis y_axis_plane = { 960, 30 };
	Axis z_axis_plane = {   0, 30 };
	SDL_FRect cs_rect_plane = { 640, 0, 640, 720 };
	CoordinateSystem *cs_plane = new CoordinateSystem(
		x_axis_plane,
		y_axis_plane,
		z_axis_plane,
		cs_rect_plane
	);
	Vector2 sample(-1, -2);

	DrawWindow *window = new DrawWindow(1280, 720);

	Scene *scene_sph   = window->add_scene(cs_sphere);
	Scene *scene_plane = window->add_scene(cs_plane);

	scene_sph->spheres.push_back(sph1);
	scene_sph->spheres.push_back(sph2);
	scene_sph->spheres.push_back(sph3);
	scene_sph->light_sources.push_back(light);

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

		scene_sph->blit_bg(CLR_VOID);
		scene_sph->render_with_ambient_diffusion_and_specular_light(&camera);
		scene_sph->blit_light_sources(&camera, 6, true);
		scene_sph->blit_axes_3d(&camera, 6.0);

		//scene_sph->light_sources[0].rotate_xz(M_PI / 96);
		camera.dir.rotate_xz(M_PI / 96);
		camera.pos.rotate_xz(M_PI / 96);

		scene_plane->blit_bg(CLR_WHITE);
		scene_plane->blit_grid();
		scene_plane->blit_axes();
		scene_plane->blit_vector(sample);
		sample.rotate(-M_PI / 96);

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
