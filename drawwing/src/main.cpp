#include <cmath>

#include "draww/vector.hpp"
#include "draww/window.hpp"
#include "draww/axes.hpp"

static float fun(float x) {
	return std::sin(x) * 2;
}

static const int FPS = 60;
static const Uint64 FRAME_NS = SDL_NS_PER_SECOND / FPS;

int main() {
	SDL_Event ev;
	bool running = true;

	Axis x_axis_main = { 400, 30 };
	Axis y_axis_main = { 550, 30 };
	SDL_FRect cs_rect_main = { 200, 150, 700, 500 };
	CoordinateSystem *cs_main = new CoordinateSystem(x_axis_main, y_axis_main, cs_rect_main);

	Axis x_axis_aux = { 600, 20 };
	Axis y_axis_aux = { 1050, 20 };
	SDL_FRect cs_rect_aux = { 1000, 450, 200, 200 };
	CoordinateSystem *cs_aux = new CoordinateSystem(x_axis_aux, y_axis_aux, cs_rect_aux);

	DrawWindow *window = new DrawWindow(1280, 720);
	Vector sample = { 1, 2 };

	Uint64 next_frame = SDL_GetTicksNS();

	while (running) {
		// FPS cap
		for (;;) {
			Uint64 now = SDL_GetTicksNS();
			if (now >= next_frame) break;

			Uint32 timeout_ms = (Uint32)((next_frame - now) / SDL_NS_PER_MS);

			if (timeout_ms == 0) break;

			if (SDL_WaitEventTimeout(&ev, (int)timeout_ms)) {
				do {
					if (ev.type == SDL_EVENT_QUIT ||
							ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
						running = false;
						break;
					}
				} while (running && SDL_PollEvent(&ev));
			}
			if (!running) break;
		}
		if (!running) break;

		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_EVENT_QUIT ||
					ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
				running = false;
			}
		}
		if (!running) break;

		// Rendering

		window->clear();

		window->blit_coordinates(*cs_main);
		window->draw_func(*cs_main, fun);

		window->blit_coordinates(*cs_aux);
		window->draw_func(*cs_aux, std::cos);

		window->draw_vector(*cs_main, sample);
		rotate_vector(sample, M_PI / 64);

		window->present();

		// Tick management
		next_frame += FRAME_NS;
		Uint64 now = SDL_GetTicksNS();
		if (now > next_frame + FRAME_NS) next_frame = now;
	}

	delete window;
	return 0;
}
