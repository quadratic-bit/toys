#pragma once
#include <swuix/state.hpp>

struct OptickState : public State {
    OptickState(Window *window_) : State(window_->GetTime(), window_) {}
};
