#pragma once
#include <swuix/widget.hpp>
#include <swuix/geometry.hpp>

struct MouseState {
	enum Enum {
		Idle,
		Dragging
	};
	unsigned state;
	Point2f  pos;  // absolute
	Widget  *target;
	Widget  *capture;

	MouseState() : state(Idle), pos(), target(NULL), capture(NULL) {}
};

struct State {
	Time       now;
	MouseState mouse;

	State() : now(0), mouse() {}
	State(Time now_) : now(now_), mouse() {}
};
