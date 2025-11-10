#pragma once
#include <swuix/manager.hpp> // TODO:n't forget to remove
#include <swuix/state.hpp>
#include <swuix/window/window.hpp>

struct OptickState : public State {
    OptickState(Window *window_) : State(Windownow(), window_) {}
};
