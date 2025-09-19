#include "areactor/linalg/axes.hpp"
#include "areactor/reactor.hpp"
#include "areactor/window.hpp"
#include <cstdio>
#include <sstream>
#include <string>

static const int FPS = 60;
static const Uint64 FRAME_NS = SDL_NS_PER_SECOND / FPS;
static const double T_window = 5.0; // seconds plotted
static const double UNITS_PER_SECONDS = 1.0; // units per seconds

static bool is_ev_close(const SDL_Event *event) {
	return event->type == SDL_EVENT_QUIT ||
		event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
}

#define NS_TO_SECONDS(ns) ns / (double)1000000000

inline void on_new_sample(RingBuffer<float> *buf, View *view, size_t sample_count, AReactorWindow *window) {
	const double dx = (view->dim.w / view->x_axis.scale) / (FPS * T_window);

	double x_latest = (double)sample_count * dx;

	view->pin_to_right(x_latest);

	const size_t n = buf->size;
	const double x0_base = x_latest - (n - 1) * dx; // space x of oldest sample stored

	window->draw_line_graph_stream(view, buf, x0_base, dx);
}

int main() {
	SDL_Event ev;
	bool running = true;

	Uint64 next_frame = SDL_GetTicksNS();  // FPS management

	SDL_FRect bbox = { 160, 160, 640, 380 };
	Reactor *reactor = new Reactor(bbox, NS_TO_SECONDS(next_frame), 2000);
	AReactorWindow *window = new AReactorWindow(1280, 720, reactor);

	Axis x_axis_energy = { 300, 10 };
	Axis y_axis_energy = { 1000, 10 };
	SDL_FRect cs_rect_energy = { 900, 160, 180, 180 };
	View total_energy = View(x_axis_energy, y_axis_energy, cs_rect_energy);
	total_energy.x_axis.scale = total_energy.dim.w / T_window * UNITS_PER_SECONDS;
	RingBuffer<float> energyY(4096);

	Axis x_axis_vel = { 500, 10 };
	Axis y_axis_vel = { 1000, 10 };
	SDL_FRect cs_rect_vel = { 900, 360, 180, 180 };
	View velocity = View(x_axis_vel, y_axis_vel, cs_rect_vel);
	velocity.x_axis.scale = velocity.dim.w / T_window * UNITS_PER_SECONDS;
	RingBuffer<float> velocityY(4096);

	size_t sample_count = 0;
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
		sample_count++;

		energyY.push((float)stats.total_energy);
		total_energy.y_axis.scale = total_energy.dim.h / energyY.mean((int)((double)FPS * T_window)) / 2.0;

		window->draw_view_grid(&total_energy, 1.0);
		window->draw_view_axes(&total_energy);
		on_new_sample(&energyY, &total_energy, sample_count, window);
		window->outline(total_energy.dim, 2);

		velocityY.push((float)stats.avg_sqr_velocity);
		velocity.y_axis.scale = velocity.dim.h / velocityY.mean((int)((double)FPS * T_window)) / 2.0;

		window->draw_view_grid(&velocity, 1.0);
		window->draw_view_axes(&velocity);
		on_new_sample(&velocityY, &velocity, sample_count, window);
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
