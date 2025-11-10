#pragma once
#include <swuix/widget.hpp>
#include <dr4/window.hpp>

using dr4::Window;

class FocusableWidget;

class Mouse {
    friend struct State;
    FocusableWidget *focus;

public:
    enum class State {
        Idle,
        Dragging
    };

    State      state;
    Vec2f      pos;
    Widget    *target;
    Widget    *wheel_target;
    Widget    *capture;

    Mouse() :
        focus(nullptr), state(State::Idle), pos(0, 0),
        target(nullptr), wheel_target(nullptr), capture(nullptr) {}
};

struct State {
    Time       now;
    Mouse      mouse;
    bool       exit_requested;
    Window     *window;
    dr4::Font  *appfont;

    State(Time now_, Window *w) : now(now_), mouse(), exit_requested(false), window(w) {
        appfont = w->CreateFont();
        appfont->LoadFromFile("/usr/share/fonts/TTF/CaskaydiaCoveNerdFontMono-Regular.ttf");
    }

    void focus(FocusableWidget *target) {
        if (!mouse.focus) window->StartTextInput();
        mouse.focus = target;
    }

    void unfocus() {
        if (mouse.focus) window->StopTextInput();
        mouse.focus = nullptr;
    }

    FocusableWidget *getFocus() {
        return mouse.focus;
    }
};
