#pragma once
#include <chrono>
#include <cstdio>
#include <optional>

#include <swuix/window/common.hpp>
#include <swuix/state.hpp>
#include <swuix/traits/focusable.hpp>
#include <thread>

class EventManager {
    State *state;

    Time next_frame;
    Time last_tick;
    int  FPS;

    inline Time frameDuration() const { return 1.0 / static_cast<Time>(FPS); }

    inline Time time() {
        return state->window->GetTime();
    }

    Time advanceFrame() {
        const Time now = time();
        state->now = now;

        Time dt_s = now - last_tick;
        last_tick = now;

        static const Time DT_MAX = 0.100;
        if (dt_s > DT_MAX) dt_s = DT_MAX;
        if (dt_s < 0.0) dt_s = 0.0;
        return dt_s;
    }

    void forceSyncMousePos(Vec2f abs, Widget *root) {
        state->mouse.target = NULL;
        state->mouse.wheel_target = NULL;
        state->mouse.pos = abs;
        MouseMoveEvent we(abs);
        DispatcherCtx ctx = DispatcherCtx::fromAbsolute(abs, root->frame(), state);
        root->broadcast(ctx, &we);
        if (!state->mouse.target) state->mouse.target = root;
    }

    void syncMousePos(Vec2f abs, Widget *root) {
        if (state->mouse.pos.x == abs.x && state->mouse.pos.y == abs.y) return;
        forceSyncMousePos(abs, root);
    }

    bool processEvent(const dr4::Event &ev, Widget *root) {
        switch (ev.type) {
        case dr4::Event::Type::QUIT: {
            QuitRequestEvent e;
            DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame(), state);
            DispatchResult res = root->broadcast(ctx, &e);
            if (res == PROPAGATE) state->exit_requested = true;
            return true;
        } break;

        case dr4::Event::Type::KEY_DOWN: {
            KeyDownEvent we(ev.key.sym, ev.key.mods, false);
            if (state->getFocus()) {
                FocusableWidget *capturer = state->getFocus();
                DispatcherCtx ctx = capturer->resolveContext();
                capturer->broadcast(ctx, &we);
            } else {
                DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame(), state);
                root->broadcast(ctx, &we);
            }
        } break;

        case dr4::Event::Type::KEY_UP: {
            KeyUpEvent we(ev.key.sym, ev.key.mods);
            DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame(), state);
            root->broadcast(ctx, &we);
        } break;

        case dr4::Event::Type::TEXT_EVENT: {
            InputEvent we(ev.text.unicode);
            if (state->getFocus()) {
                FocusableWidget *capturer = state->getFocus();
                DispatcherCtx ctx = capturer->resolveContext();
                capturer->broadcast(ctx, &we);
            } else {
                DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame(), state);
                root->broadcast(ctx, &we);
            }
        } break;

        case dr4::Event::Type::MOUSE_MOVE:
            syncMousePos(Vec2f(ev.mouseMove.pos.x, ev.mouseMove.pos.y), root);
            break;

        case dr4::Event::Type::MOUSE_DOWN: {
            state->mouse.state = Mouse::State::Dragging;
            state->mouse.pos = Vec2f(ev.mouseButton.pos.x, ev.mouseButton.pos.y);
            state->unfocus();
            syncMousePos(state->mouse.pos, root);
            MouseDownEvent we(state->mouse.pos);
            if (state->mouse.capture) {
                Widget *capturer = state->mouse.capture;
                DispatcherCtx ctx = capturer->resolveContext();
                capturer->broadcast(ctx, &we);
            } else {
                DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame(), state);
                root->broadcast(ctx, &we);
            }
            if (state->getFocus()) state->getFocus()->focus();
        } break;

        case dr4::Event::Type::MOUSE_UP: {
            state->mouse.state = Mouse::State::Idle;
            state->mouse.pos = Vec2f(ev.mouseButton.pos.x, ev.mouseButton.pos.y);
            syncMousePos(state->mouse.pos, root);
            MouseUpEvent we;
            if (state->mouse.capture) {
                Widget *capturer = state->mouse.capture;
                DispatcherCtx ctx = capturer->resolveContext();
                capturer->broadcast(ctx, &we);
            } else {
                DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame(), state);
                root->broadcast(ctx, &we);
            }
        } break;

        case dr4::Event::Type::MOUSE_WHEEL: {
            state->mouse.pos = Vec2f(ev.mouseWheel.pos.x, ev.mouseWheel.pos.y);
            syncMousePos(state->mouse.pos, root);
            Vec2f scrolled = Vec2f(0, ev.mouseWheel.delta.y);
            MouseWheelEvent we(scrolled);
            if (!state->mouse.wheel_target) break;
            Widget *capturer = state->mouse.wheel_target;
            DispatcherCtx ctx = capturer->resolveContext();
            capturer->broadcast(ctx, &we);
            forceSyncMousePos(state->mouse.pos, root);
        } break;

        case dr4::Event::Type::UNKNOWN: break;

        } // switch(ev.type)

        return state->exit_requested;
    }

public:
    EventManager(State *state_, int fps) : state(state_), FPS(fps) {
        const Time f = frameDuration();
        const Time now = time();
        last_tick = now;
        next_frame = now + f;
    }

    void render(Widget *root) {
        state->window->Clear(Color(CLR_BACKGROUND, 255));
        root->blit(root->texture);
        state->window->Draw(*root->texture);
        state->window->Display();
    }

    void prepareEvents() {
        const Time f = frameDuration();
        next_frame += f;
        const Time now = time();

        if (now > next_frame + f) {
            const Time behind = now - next_frame;
            const Time missed = std::floor(behind / f);
            next_frame += missed * f;
            if (next_frame < now) next_frame = now + f;
        }
    }

    void idle(Widget *root) {
        const Time dt_s = advanceFrame();
        const Time now = time();

        // timer granularity
        static const Time SAFETY_MARGIN_S = 0.001;

        Time remaining_s = (next_frame - now) - SAFETY_MARGIN_S;
        if (remaining_s < 0.0) remaining_s = 0.0;

        const Time deadline = next_frame - SAFETY_MARGIN_S;

        DispatcherCtx ctx = DispatcherCtx::fromAbsolute(state->mouse.pos, root->frame(), state);
        IdleEvent idle_e(dt_s, remaining_s, deadline);

        root->broadcast(ctx, &idle_e);
    }

    bool exhaustEvents(Widget *root) {
        std::optional<dr4::Event> ev;

        const Time now = time();
        state->now = now;
        if (now >= next_frame) return true; // deadline reached

        const Time remaining_s = next_frame - now;
        int64_t remaining_ms = static_cast<int64_t>(remaining_s * 1e3);
        if (remaining_ms <= 0) return true;

        static const int MAX_EVENT_WAIT_MS = 3;       // max wait per iteration
        static const int POLL_ONLY_THRESHOLD_MS = 2;  // close to deadline -> only poll

        if (remaining_ms <= POLL_ONLY_THRESHOLD_MS) {
            while ((ev = state->window->PollEvent())) {
                if (ev == std::nullopt) return false;
                if (processEvent(ev.value(), root)) return true;
                if (state->exit_requested) return true;
            }
            return false;
        }

        const int wait_ms = static_cast<int>(std::min<int64_t>(remaining_ms, MAX_EVENT_WAIT_MS));
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));

        // drain anything else already queued
        while ((ev = state->window->PollEvent())) {
            if (ev == std::nullopt) return false;
            if (processEvent(ev.value(), root)) return true;
            if (state->exit_requested) return true;
        }

        return false;
    }
};
