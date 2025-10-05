#pragma once
#include <swuix/geometry.hpp>
#include <swuix/widget.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

class EventManager {
	State *state;

	Time next_frame;
	Time last_tick;
	int FPS;

	static bool is_ev_close(const SDL_Event *event) {
		return event->type == SDL_EVENT_QUIT ||
			event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
	}

	inline Time frame_dur() const { return (Time)1 / (Time)FPS; }

	void ensure_latest_mouse_pos(Point2f abs, Widget *root) {
		if (state->mouse.pos == abs) return;
		state->mouse.target = NULL;
		state->mouse.pos = abs;
		MouseMoveEvent we(abs);
		DispatcherCtx ctx = DispatcherCtx::from_absolute(abs, root->frame);
		root->route(ctx, &we);
	}

public:
	EventManager(State *state_, int fps) : state(state_), FPS(fps) {
		next_frame = Window::now();
		last_tick = next_frame;
	}

	void prepare_events() {
		next_frame += frame_dur();
		Time now = Window::now();
		if (now > next_frame + frame_dur()) next_frame = now;
		SDL_PumpEvents();
	}

	/*
	 * Update the current time in state and return delta time
	 * since the last call of this function.
	 */
	Time advance_frame() {
		Time now = Window::now();
		state->now = now;

		Time dt_s = now - last_tick;
		last_tick = now;
		static const Time DT_MAX = 0.100;  // 100 ms
		if (dt_s > DT_MAX) dt_s = DT_MAX;
		return dt_s;
	}

	void dispatch_idle(Widget *root) {
		double dt_s = advance_frame();

		IdleEvent idle_e(dt_s);
		DispatcherCtx ctx = DispatcherCtx::from_absolute(state->mouse.pos, root->frame);
		root->route(ctx, &idle_e);
	}

	bool exhaust_events(Widget *root) {
		SDL_Event ev;

		Time now = Window::now();
		state->now = now;
		if (now >= next_frame) return true;

		Time timeout_ms = (next_frame - now) * 1e3;

		if (timeout_ms < 0) return true;

		if (!SDL_WaitEventTimeout(&ev, (int32_t)timeout_ms)) return false;

		do {
			switch (ev.type) {
			case SDL_EVENT_QUIT:
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
				QuitRequestEvent e;
				DispatcherCtx ctx = DispatcherCtx::from_absolute(state->mouse.pos, root->frame);
				DispatchResult res = root->route(ctx, &e);
				if (res == PROPAGATE) state->exit_requested = true;
				return true;
				} break;
			case SDL_EVENT_KEY_DOWN: {
				KeyDownEvent we(ev.key.scancode, ev.key.key, ev.key.mod, ev.key.repeat);
				DispatcherCtx ctx = DispatcherCtx::from_absolute(state->mouse.pos, root->frame);
				root->route(ctx, &we);
				} break;
			case SDL_EVENT_KEY_UP: {
				KeyUpEvent we(ev.key.scancode, ev.key.key, ev.key.mod);
				DispatcherCtx ctx = DispatcherCtx::from_absolute(state->mouse.pos, root->frame);
				root->route(ctx, &we);
				} break;
			case SDL_EVENT_MOUSE_MOTION:
				ensure_latest_mouse_pos(Point2f(ev.motion.x, ev.motion.y), root);
				break;
			case SDL_EVENT_MOUSE_BUTTON_DOWN: {
				state->mouse.state = MouseState::Dragging;
				state->mouse.pos = Point2f(ev.button.x, ev.button.y);
				ensure_latest_mouse_pos(state->mouse.pos, root);
				MouseDownEvent we(state->mouse.pos);
				if (state->mouse.capture) {
					Widget *capturer = state->mouse.capture;
					DispatcherCtx ctx = capturer->resolve_capture_context();
					capturer->route(ctx, &we);
				} else {
					DispatcherCtx ctx = DispatcherCtx::from_absolute(state->mouse.pos, root->frame);
					root->route(ctx, &we);
				}
				} break;
			case SDL_EVENT_MOUSE_BUTTON_UP: {
				state->mouse.state = MouseState::Idle;
				state->mouse.pos = Point2f(ev.button.x, ev.button.y);
				ensure_latest_mouse_pos(state->mouse.pos, root);
				MouseUpEvent we;
				if (state->mouse.capture) {
					Widget *capturer = state->mouse.capture;
					DispatcherCtx ctx = capturer->resolve_capture_context();
					capturer->route(ctx, &we);
				} else {
					DispatcherCtx ctx = DispatcherCtx::from_absolute(state->mouse.pos, root->frame);
					root->route(ctx, &we);
				}
				} break;
			}
		} while (!state->exit_requested && SDL_PollEvent(&ev));
		return false;
	}
};
