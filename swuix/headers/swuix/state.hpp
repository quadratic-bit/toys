#pragma once
#include <swuix/widget.hpp>
#include <swuix/geometry.hpp>

class FocusableWidget;  // forward-declare

struct MouseState {
    enum Enum {
        Idle,
        Dragging
    };
    unsigned state;
    Vec2F    pos;  // absolute
    Widget  *target;
    Widget  *wheel_target;
    Widget  *capture;

    FocusableWidget *focus;

    MouseState() : state(Idle), pos(), target(NULL), wheel_target(NULL), capture(NULL), focus(NULL) {}
};

struct State {
    Time       now;
    MouseState mouse;
    bool       exit_requested;
    Window     *window;

    State(Window *window_) : now(0), mouse(), exit_requested(false), window(window_) {}
    State(Time now_, Window *window_) : now(now_), mouse(), exit_requested(false), window(window_) {}
};
