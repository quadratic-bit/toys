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

	inline Time frame_dur() const { return 1.0 / static_cast<Time>(FPS); }

	void ensure_latest_mouse_pos(Point2f abs, Widget *root) {
		if (state->mouse.pos == abs) return;
		state->mouse.target = NULL;
		state->mouse.pos = abs;
		MouseMoveEvent we(abs);
		DispatcherCtx ctx = DispatcherCtx::from_absolute(abs, root->frame);
		root->route(ctx, &we);
		if (!state->mouse.target) state->mouse.target = root;
	}

public:
	EventManager(State *state_, int fps) : state(state_), FPS(fps) {
		const Time f = frame_dur();
		const Time now = Window::now();
		last_tick = now;
		next_frame = now + f;
	}

	void prepare_events() {
		const Time f = frame_dur();
		next_frame += f;
		const Time now = Window::now();

		if (now > next_frame + f) {
			const Time behind = now - next_frame;
			const Time missed = std::floor(behind / f);
			next_frame += missed * f;
			if (next_frame < now) next_frame = now + f;
		}
	}

	/*
	 * Update the current time in state and return delta time
	 * since the last call of this function.
	 */
	Time advance_frame() {
		const Time now = Window::now();
		state->now = now;

		Time dt_s = now - last_tick;
		last_tick = now;

		static const Time DT_MAX = 0.100;
		if (dt_s > DT_MAX) dt_s = DT_MAX;
		if (dt_s < 0.0) dt_s = 0.0;
		return dt_s;
	}

	void dispatch_idle(Widget *root) {
		const Time dt_s = advance_frame();
		const Time now = Window::now();

		// timer granularity
		static const Time SAFETY_MARGIN_S = 0.001;

		Time remaining_s = (next_frame - now) - SAFETY_MARGIN_S;
		if (remaining_s < 0.0) remaining_s = 0.0;

		const Time deadline = next_frame - SAFETY_MARGIN_S;

		DispatcherCtx ctx = DispatcherCtx::from_absolute(state->mouse.pos, root->frame);
		IdleEvent idle_e(dt_s, remaining_s, deadline);

		root->route(ctx, &idle_e);
	}

	bool handle_event(const SDL_Event &ev, Widget *root) {
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
		return state->exit_requested;
	}

	/* true if deadline reached or we exit requested */
	bool exhaust_events(Widget *root) {
		SDL_Event ev;

		const Time now = Window::now();
		state->now = now;
		if (now >= next_frame) return true; // deadline reached

		const Time remaining_s = next_frame - now;
		int64_t remaining_ms = static_cast<int64_t>(remaining_s * 1e3);
		if (remaining_ms <= 0) return true;

		static const int MAX_EVENT_WAIT_MS = 3;       // max wait per iteration
		static const int POLL_ONLY_THRESHOLD_MS = 2;  // close to deadline -> only poll

		if (remaining_ms <= POLL_ONLY_THRESHOLD_MS) {
			while (SDL_PollEvent(&ev)) {
				if (handle_event(ev, root)) return true;
				if (state->exit_requested) return true;
			}
			return false;
		}

		const int wait_ms = static_cast<int>(std::min<int64_t>(remaining_ms, MAX_EVENT_WAIT_MS));
		if (!SDL_WaitEventTimeout(&ev, wait_ms)) {
			// timeout without events — don't keep blocking
			return false;
		}

		if (handle_event(ev, root)) return true;

		// drain anything else already queued
		while (SDL_PollEvent(&ev)) {
			if (handle_event(ev, root)) return true;
		}

		return false;
	}
};
