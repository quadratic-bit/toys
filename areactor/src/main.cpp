#include <SDL3/SDL_events.h>
#include <cassert>
#include <cstdio>

#include <swuix/window/window.hpp>
#include <swuix/widgets/toolbox.hpp>

#include "areactor/graph.hpp"
#include "areactor/reactor.hpp"
#include "areactor/state.hpp"

static const int FPS = 60;
static const Uint64 FRAME_NS = SDL_NS_PER_SECOND / FPS;

static bool is_ev_close(const SDL_Event *event) {
	return event->type == SDL_EVENT_QUIT ||
		event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
}

static void cb_add(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_speed(+40);
}

static void cb_sub(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_speed(-40);
}

static void cb_add_particles(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->reactor->add_particles(((ReactorState*)st)->now, 5);
}

static void cb_delete_particles(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->reactor->remove_particles(5);
}

static void cb_hotter(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_temp(Side::LEFT, +0.50);
}

static void cb_colder(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_temp(Side::LEFT, -0.50);
}

#define NS_TO_SECONDS(ns) (double)ns / (double)1000000000

int main() {
	SDL_Event ev;
	bool running = true;
	ReactorState state = ReactorState();
	SDL_FRect dim = frect(0, 0, 1280, 720);
	WidgetContainer root(dim, NULL, &state);
	root.parent = &root;

	Uint64 next_frame = SDL_GetTicksNS();  // FPS management

	SDL_FRect bbox = frect(160, 160, 640, 380);
	Reactor *reactor = new Reactor(bbox, NULL, 1000, &state);
	state.now = NS_TO_SECONDS(next_frame);
	state.reactor = reactor;
	Window *window = new Window(dim.w, dim.h);

	SDL_FRect cs_rect_energy = frect(1000, 140, 180, 180);
	LineGraph kinetic = LineGraph(cs_rect_energy, NULL, "J", "E", &state, 0.8, FPS, 5.0);

	SDL_FRect cs_rect_vel = frect(1000, 380, 180, 180);
	LineGraph temperature = LineGraph(cs_rect_vel, NULL, "K", "T", &state, 0.8, FPS, 5.0);

	Button btn_left(frect(20, 10, 50, 25), NULL, "<-", &state, cb_sub);
	Button btn_right(frect(80, 10, 50, 25), NULL, "->", &state, cb_add);

	Button btn_cold(frect(20, 40, 50, 25), NULL, "Cold", &state, cb_colder);
	Button btn_hot(frect(80, 40, 50, 25), NULL, "Hot",  &state, cb_hotter);

	Button btn_add(frect(20, 70, 50, 25), NULL, "+", &state, cb_add_particles);
	Button btn_remove(frect(80, 70, 50, 25), NULL, "-",  &state, cb_delete_particles);

	SDL_FRect toolbox_rect = frect(10, 30, 150, 110);
	Widget *btns[] = { &btn_left, &btn_right, &btn_cold, &btn_hot, &btn_add, &btn_remove };
	ToolboxWidget toolbox(toolbox_rect, NULL, Widget::make_children(btns), &state);

	Widget *arr[] = { reactor, &toolbox, &kinetic, &temperature };
	root.append_children(Widget::make_children(arr));

	Stat stats;

	static Uint64 last_ticks_ns = 0;
	while (running) {
		SDL_PumpEvents();
		// FPS cap
		for (;;) {
			Uint64 now = SDL_GetTicksNS();
			state.now = NS_TO_SECONDS(now);
			if (now >= next_frame) break;

			Uint32 timeout_ms = (next_frame - now) / SDL_NS_PER_MS;

			if (timeout_ms == 0) break;

			if (SDL_WaitEventTimeout(&ev, timeout_ms)) {
				do {
					if (is_ev_close(&ev)) {
						running = false;
						break;
					}
					switch (ev.type) {
					case SDL_EVENT_KEY_DOWN:
						state.add_particle |= (ev.key.scancode == SDL_SCANCODE_N);
						state.delete_particle |= (ev.key.scancode == SDL_SCANCODE_D);
						if (ev.key.scancode == SDL_SCANCODE_RIGHT) {
							state.wall_speed += 40;
							state.wall_speed_changed = true;
						}
						if (ev.key.scancode == SDL_SCANCODE_LEFT) {
							state.wall_speed -= 40;
							state.wall_speed_changed = true;
						}
						break;
					case SDL_EVENT_KEY_UP:
						state.add_particle &= !(ev.key.scancode == SDL_SCANCODE_N);
						state.delete_particle &= !(ev.key.scancode == SDL_SCANCODE_D);
						break;
					case SDL_EVENT_MOUSE_MOTION: {
						state.mouse.target = NULL;
						MouseMoveEvent we(ev.motion.x, ev.motion.y);
						DispatcherCtx ctx = DispatcherCtx::from_absolute(we.ax, we.ay);
						root.route(ctx, &we);
						} break;
					case SDL_EVENT_MOUSE_BUTTON_DOWN: {
						MouseDownEvent we(ev.button.x, ev.button.y);
						state.mouse.state = MouseState::Dragging;
						DispatcherCtx ctx = DispatcherCtx::from_absolute(we.ax, we.ay);
						// TODO PERF: directly resolve relative coords and dispatch?
						root.route(ctx, &we);
						} break;
					case SDL_EVENT_MOUSE_BUTTON_UP: {
						MouseUpEvent we;
						state.mouse.state = MouseState::Idle;
						DispatcherCtx ctx = DispatcherCtx::from_absolute(ev.button.x, ev.button.y);
						root.route(ctx, &we);
						} break;
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
		if (last_ticks_ns == 0) last_ticks_ns = ticks_now;

		Uint64 dt_ns = ticks_now - last_ticks_ns;
		last_ticks_ns = ticks_now;

		double start_s = NS_TO_SECONDS(ticks_now);
		double dt_s    = NS_TO_SECONDS(dt_ns);
		state.now      = start_s;

		if (state.add_particle) reactor->add_particles(reactor->sim_now, 2);
		if (state.delete_particle) reactor->remove_particle();

		if (state.wall_speed_changed) {
			reactor->set_right_wall_velocity(state.wall_speed);
			state.wall_speed_changed = false;
		}

		// Rendering
		window->clear();

		stats = reactor->tally();

		kinetic.append_sample(stats.kinetic);
		kinetic.rescale_y();
		kinetic.snap_y_scale_to_grid();

		temperature.append_sample(stats.right_temperature);
		temperature.rescale_y();
		temperature.snap_y_scale_to_grid();

		root.render(window, 0, 0);

		window->present();

		if (dt_s > 0.050) dt_s = 0.050;
		reactor->step_frame(start_s, dt_s);

		// Tick management
		next_frame += FRAME_NS;
		Uint64 now = SDL_GetTicksNS();
		if (now > next_frame + FRAME_NS) next_frame = now;
	}

	delete window;
	return 0;
}
