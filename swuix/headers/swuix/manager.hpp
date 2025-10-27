#pragma once
#include <swuix/geometry.hpp>
#include <swuix/widget.hpp>
#include <swuix/widgets/focusable.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

class EventManager {
    State *state;

    Time next_frame;
    Time last_tick;
    int  FPS;

public:
    EventManager(State *state_, int fps) : state(state_), FPS(fps) {
        const Time f = frameDuration();
        const Time now = Window::now();
        last_tick = now;
        next_frame = now + f;
    }

    void render(Widget *root) {
        state->window->clear();
        RenderEvent e;
        DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame, state->window);
        root->broadcast(ctx, &e, true);
        state->window->present();
    }

    void prepareEvents() {
        const Time f = frameDuration();
        next_frame += f;
        const Time now = Window::now();

        if (now > next_frame + f) {
            const Time behind = now - next_frame;
            const Time missed = std::floor(behind / f);
            next_frame += missed * f;
            if (next_frame < now) next_frame = now + f;
        }
    }

    void idle(Widget *root) {
        const Time dt_s = advanceFrame();
        const Time now = Window::now();

        // timer granularity
        static const Time SAFETY_MARGIN_S = 0.001;

        Time remaining_s = (next_frame - now) - SAFETY_MARGIN_S;
        if (remaining_s < 0.0) remaining_s = 0.0;

        const Time deadline = next_frame - SAFETY_MARGIN_S;

        DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame, state->window);
        IdleEvent idle_e(dt_s, remaining_s, deadline);

        root->broadcast(ctx, &idle_e);
    }

    /* true if deadline reached or we exit requested */
    bool exhaustEvents(Widget *root) {
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
                if (processEvent(ev, root)) return true;
                if (state->exit_requested) return true;
            }
            return false;
        }

        const int wait_ms = static_cast<int>(std::min<int64_t>(remaining_ms, MAX_EVENT_WAIT_MS));
        if (!SDL_WaitEventTimeout(&ev, wait_ms)) {
            // timeout without events — don't keep blocking
            return false;
        }

        if (processEvent(ev, root)) return true;

        // drain anything else already queued
        while (SDL_PollEvent(&ev)) {
            if (processEvent(ev, root)) return true;
        }

        return false;
    }

private:
    static bool isEventClose(const SDL_Event *event) {
        return event->type == SDL_EVENT_QUIT ||
            event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED;
    }

    inline Time frameDuration() const { return 1.0 / static_cast<Time>(FPS); }

    void syncMousePos(Vec2F abs, Widget *root) {
        if (state->mouse.pos == abs) return;
        state->mouse.target = NULL;
        state->mouse.wheel_target = NULL;
        state->mouse.pos = abs;
        MouseMoveEvent we(abs);
        DispatcherCtx ctx = DispatcherCtx::fromAbsolute(abs, root->frame, state->window);
        root->broadcast(ctx, &we);
        if (!state->mouse.target) state->mouse.target = root;
    }

    /*
     * Update the current time in state and return delta time
     * since the last call of this function.
     */
    Time advanceFrame() {
        const Time now = Window::now();
        state->now = now;

        Time dt_s = now - last_tick;
        last_tick = now;

        static const Time DT_MAX = 0.100;
        if (dt_s > DT_MAX) dt_s = DT_MAX;
        if (dt_s < 0.0) dt_s = 0.0;
        return dt_s;
    }

    bool processEvent(const SDL_Event &ev, Widget *root) {
        switch (ev.type) {

        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
            QuitRequestEvent e;
            DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame, state->window);
            DispatchResult res = root->broadcast(ctx, &e);
            if (res == PROPAGATE) state->exit_requested = true;
            return true;
        } break;

        case SDL_EVENT_KEY_DOWN: {
            KeyDownEvent we(ev.key.scancode, ev.key.key, ev.key.mod, ev.key.repeat);
            if (state->mouse.focus) {
                Widget *capturer = state->mouse.focus;
                DispatcherCtx ctx = capturer->resolve_context(state->window);
                capturer->broadcast(ctx, &we);
            } else {
                DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame, state->window);
                root->broadcast(ctx, &we);
            }
        } break;

        case SDL_EVENT_KEY_UP: {
            KeyUpEvent we(ev.key.scancode, ev.key.key, ev.key.mod);
            DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame, state->window);
            root->broadcast(ctx, &we);
        } break;

        case SDL_EVENT_MOUSE_MOTION:
            syncMousePos(Vec2F(ev.motion.x, ev.motion.y), root);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            state->mouse.state = MouseState::Dragging;
            state->mouse.pos = Vec2F(ev.button.x, ev.button.y);
            state->mouse.focus = NULL;
            syncMousePos(state->mouse.pos, root);
            MouseDownEvent we(state->mouse.pos);
            if (state->mouse.capture) {
                Widget *capturer = state->mouse.capture;
                DispatcherCtx ctx = capturer->resolve_context(state->window);
                capturer->broadcast(ctx, &we);
            } else {
                DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame, state->window);
                root->broadcast(ctx, &we);
            }
            if (state->mouse.focus) state->mouse.focus->focus();
        } break;

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            state->mouse.state = MouseState::Idle;
            state->mouse.pos = Vec2F(ev.button.x, ev.button.y);
            syncMousePos(state->mouse.pos, root);
            MouseUpEvent we;
            if (state->mouse.capture) {
                Widget *capturer = state->mouse.capture;
                DispatcherCtx ctx = capturer->resolve_context(state->window);
                capturer->broadcast(ctx, &we);
            } else {
                DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame, state->window);
                root->broadcast(ctx, &we);
            }
        } break;

        case SDL_EVENT_MOUSE_WHEEL: {
            state->mouse.pos = Vec2F(ev.wheel.mouse_x, ev.wheel.mouse_y);
            syncMousePos(state->mouse.pos, root);
            Vec2F scrolled = Vec2F(ev.wheel.x, ev.wheel.y);
            MouseWheelEvent we(scrolled);
            if (!state->mouse.wheel_target) break;
            Widget *capturer = state->mouse.wheel_target;
            DispatcherCtx ctx = capturer->resolve_context(state->window);
            capturer->broadcast(ctx, &we);
        } break;

        } // switch(ev.type)

        return state->exit_requested;
    }
};
