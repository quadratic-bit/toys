#pragma once
#include "widget.hpp"

struct MouseState {
	enum Enum {
		Idle,
		Dragging
	};
	unsigned state;
	Widget *target;
	Widget *capture;

	MouseState() : state(Idle), target(NULL), capture(NULL) {}
};

struct State {
	MouseState mouse;
	State() : mouse() {}
};
