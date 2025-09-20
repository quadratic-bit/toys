#include <sstream>
#include <string>

#include "areactor/window.hpp"
#include "areactor/reactor.hpp"
#include "areactor/graph.hpp"

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

	SDL_FRect bbox = { 160, 160, 640, 380 };
	Reactor *reactor = new Reactor(bbox, NS_TO_SECONDS(next_frame), 2000);
	AReactorWindow *window = new AReactorWindow(1280, 720, reactor);

	SDL_FRect cs_rect_energy = { 1000, 160, 180, 180 };
	LineGraph total_energy = LineGraph(cs_rect_energy, 0.8, FPS, 5.0);

	SDL_FRect cs_rect_vel = { 1000, 360, 180, 180 };
	LineGraph velocity = LineGraph(cs_rect_vel, 0.8, FPS, 5.0);

	Stat stats;

	int wall_speed = 0;
	bool wall_speed_changed = false;

	bool add_particle = false;
	bool delete_particle = false;

	while (running) {
		 SDL_PumpEvents();
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
					if (ev.type == SDL_EVENT_KEY_DOWN) {
						add_particle |= (ev.key.scancode == SDL_SCANCODE_N);
						delete_particle |= (ev.key.scancode == SDL_SCANCODE_D);
						if (ev.key.scancode == SDL_SCANCODE_RIGHT) {
							wall_speed += 40;
							wall_speed_changed = true;
						}
						if (ev.key.scancode == SDL_SCANCODE_LEFT) {
							wall_speed -= 40;
							wall_speed_changed = true;
						}
					}
					if (ev.type == SDL_EVENT_KEY_UP) {
						add_particle &= !(ev.key.scancode == SDL_SCANCODE_N);
						delete_particle &= !(ev.key.scancode == SDL_SCANCODE_D);
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

		Uint64 ticks_now = SDL_GetTicksNS();
		Uint64 dt_ns = ticks_now - ticks_now;

		double start_s = NS_TO_SECONDS(ticks_now);
		double dt_s    = NS_TO_SECONDS(dt_ns);

		if (add_particle) reactor->add_particles(start_s, 2);
		if (delete_particle) reactor->remove_particle();

		if (wall_speed_changed) {
			reactor->set_right_wall_velocity(start_s, wall_speed);
			wall_speed_changed = false;
		}

		// Rendering
		window->clear();
		window->outline(reactor->bbox);
		window->draw_particles();

		stats = reactor->tally();

		total_energy.append_sample(stats.total_energy);
		total_energy.rescale_y();
		total_energy.snap_y_scale_to_grid();

		total_energy.draw_view_grid(window, 1.0, "px²/s²");
		total_energy.draw_view_axes(window->renderer);
		total_energy.plot_stream(window->renderer);
		total_energy.draw_axis_titles(window, "", "E");
		window->outline(total_energy.dim, 2);

		velocity.append_sample(stats.avg_sqr_velocity);
		velocity.rescale_y();
		velocity.snap_y_scale_to_grid();

		velocity.draw_view_grid(window, 1.0, "px²/s²");
		velocity.draw_view_axes(window->renderer);
		velocity.plot_stream(window->renderer);
		velocity.draw_axis_titles(window, "", "<v²>");
		window->outline(velocity.dim, 2);

		std::ostringstream oss;

		oss << stats.n_circle;
		std::string n_circles = oss.str();
		oss.str("");

		n_circles.append(" circle");
		window->text(n_circles.c_str(), reactor->bbox.x, reactor->bbox.y + reactor->bbox.h);

		oss << stats.n_square;
		std::string n_square = oss.str();

		n_square.append(" squares");
		window->text(n_square.c_str(), reactor->bbox.x, reactor->bbox.y + reactor->bbox.h + 20);

		window->present();

		reactor->step_frame(start_s, dt_s);

		// Tick management
		next_frame += FRAME_NS;
		Uint64 now = SDL_GetTicksNS();
		if (now > next_frame + FRAME_NS) next_frame = now;
	}

	delete window;
	return 0;
}
