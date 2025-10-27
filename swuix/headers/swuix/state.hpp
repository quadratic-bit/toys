#pragma once
#include <swuix/widget.hpp>
#include <swuix/geometry.hpp>

class FocusableWidget;  // forward-declare

class MouseState {
    friend struct State;
    FocusableWidget *focus;

public:
    enum Enum {
        Idle,
        Dragging
    };
    unsigned state;
    Vec2F    pos;  // absolute
    Widget  *target;
    Widget  *wheel_target;
    Widget  *capture;

    MouseState() : focus(NULL), state(Idle), pos(), target(NULL), wheel_target(NULL), capture(NULL) {}
};

struct State {
    Time       now;
    MouseState mouse;
    bool       exit_requested;
    Window     *window;

    State(Window *window_) : now(0), mouse(), exit_requested(false), window(window_) {}
    State(Time now_, Window *window_) : now(now_), mouse(), exit_requested(false), window(window_) {}

    void focus(FocusableWidget *target) {
        if (!mouse.focus) window->start_input();
        mouse.focus = target;
    }

    void unfocus() {
        if (mouse.focus) window->stop_input();
        mouse.focus = NULL;
    }

    FocusableWidget *get_focus() {
        return mouse.focus;
    }
};
