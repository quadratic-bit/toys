#pragma once
#include <swuix/state.hpp>

struct OptickState : public State {
    OptickState(Window *window_, dr4::Font *font) : State(window_, font) {}
};
