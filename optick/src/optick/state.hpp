#pragma once
#include <swuix/state.hpp>
#include <swuix/window/window.hpp>

struct OptickState : public State {
    OptickState(Window *window_) : State(window_) {}
};
