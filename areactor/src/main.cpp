#include "areactor/reactor.hpp"
#include "areactor/window.hpp"

static const int FPS = 60;
static const Uint64 FRAME_NS = SDL_NS_PER_SECOND / FPS;

static bool is_ev_close(const SDL_Event *event) {
	return event->type == SDL_EVENT_QUIT ||
		event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
}

#define NS_TO_SECONDS(ns) ns / (double)1000000000

int main() {
	SDL_Event ev;
	bool running = true;

	Uint64 next_frame = SDL_GetTicksNS();  // FPS management

	SDL_FRect bbox = { 320, 180, 640, 360 };
	Reactor *reactor = new Reactor(bbox, NS_TO_SECONDS(next_frame));
	AReactorWindow *window = new AReactorWindow(1280, 720, reactor);

	Uint64 timestamp = next_frame;  // physcics time calculation

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

		Uint64 _tmp = SDL_GetTicksNS();
		Uint64 dt_ns = _tmp - timestamp;
		timestamp = _tmp;

		// Rendering
		window->clear();
		window->outline_reactor();
		window->draw_particles();
		window->present();

		reactor->step_frame(NS_TO_SECONDS(timestamp), NS_TO_SECONDS(dt_ns));

		// Tick management
		next_frame += FRAME_NS;
		Uint64 now = SDL_GetTicksNS();
		if (now > next_frame + FRAME_NS) next_frame = now;
	}

	delete window;
	return 0;
}
