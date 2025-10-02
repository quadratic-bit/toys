#pragma once
#include <SDL3/SDL_events.h>
#include <cstdio>

#include <swuix/widget.hpp>

#include "state.hpp"

#define NS_TO_SECONDS(ns) (double)ns / (double)1000000000

class EventManager {
	ReactorState *state;

	Uint64 next_frame;
	Uint64 last_ticks_ns;
	int FPS;

	static bool is_ev_close(const SDL_Event *event) {
		return event->type == SDL_EVENT_QUIT ||
			event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
	}

	inline Uint64 FRAME_NS() const { return SDL_NS_PER_SECOND / FPS; }

public:
	EventManager(ReactorState *state_, int fps) : state(state_), next_frame(SDL_GetTicksNS()), FPS(fps) {
		next_frame = SDL_GetTicksNS();
		last_ticks_ns = next_frame;
	}

	void prepare_events() {
		next_frame += FRAME_NS();
		Uint64 now = SDL_GetTicksNS();
		if (now > next_frame + FRAME_NS()) next_frame = now;
		SDL_PumpEvents();
	}

	double dt_seconds() {
		Uint64 ticks_now = SDL_GetTicksNS();
		state->now = NS_TO_SECONDS(ticks_now);

		Uint64 dt_ns = ticks_now - last_ticks_ns;
		last_ticks_ns = ticks_now;
		double dt_s = NS_TO_SECONDS(dt_ns);
		const double DT_MAX = 0.1;  // 100 ms
		if (dt_s > DT_MAX) dt_s = DT_MAX;
		return dt_s;
	}

	void dispatch_idle(Widget *root) {
		double dt_s = dt_seconds();

		IdleEvent idle_e(dt_s);
		DispatcherCtx ctx = DispatcherCtx::from_absolute(0, 0);  // TODO: reuse old mouse
		root->route(ctx, &idle_e);
	}

	bool exhaust_events(Widget *root) {
		SDL_Event ev;

		Uint64 now = SDL_GetTicksNS();
		state->now = NS_TO_SECONDS(now);
		if (now >= next_frame) return true;

		Uint32 timeout_ms = (next_frame - now) / SDL_NS_PER_MS;

		if (timeout_ms == 0) return true;

		if (!SDL_WaitEventTimeout(&ev, timeout_ms)) return false;

		do {
			if (is_ev_close(&ev)) {
				state->running = false;
				return true;
			}
			switch (ev.type) {
			case SDL_EVENT_KEY_DOWN:
				state->add_particle |= (ev.key.scancode == SDL_SCANCODE_N);
				state->delete_particle |= (ev.key.scancode == SDL_SCANCODE_D);
				if (ev.key.scancode == SDL_SCANCODE_RIGHT) {
					state->wall_speed += 40;
					state->wall_speed_changed = true;
				}
				if (ev.key.scancode == SDL_SCANCODE_LEFT) {
					state->wall_speed -= 40;
					state->wall_speed_changed = true;
				}
				break;
			case SDL_EVENT_KEY_UP:
				state->add_particle &= !(ev.key.scancode == SDL_SCANCODE_N);
				state->delete_particle &= !(ev.key.scancode == SDL_SCANCODE_D);
				break;
			case SDL_EVENT_MOUSE_MOTION: {
				state->mouse.target = NULL;
				MouseMoveEvent we(ev.motion.x, ev.motion.y);
				DispatcherCtx ctx = DispatcherCtx::from_absolute(we.ax, we.ay);
				root->route(ctx, &we);
				} break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN: {
				MouseDownEvent we(ev.button.x, ev.button.y);
				state->mouse.state = MouseState::Dragging;
				DispatcherCtx ctx = DispatcherCtx::from_absolute(we.ax, we.ay);
				// TODO PERF: directly resolve relative coords and dispatch?
				root->route(ctx, &we);
				} break;
			case SDL_EVENT_MOUSE_BUTTON_UP: {
				MouseUpEvent we;
				state->mouse.state = MouseState::Idle;
				DispatcherCtx ctx = DispatcherCtx::from_absolute(ev.button.x, ev.button.y);
				root->route(ctx, &we);
				} break;
			}
		} while (state->running && SDL_PollEvent(&ev));
		return false;
	}
};
