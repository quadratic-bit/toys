#pragma once
#include "reactor.hpp"
#include "widgets/widget.hpp"

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
	int wall_speed;
	bool wall_speed_changed;

	bool add_particle;
	bool delete_particle;

	MouseState mouse;

	Reactor *reactor;

	Time now;

	State() : wall_speed(0), wall_speed_changed(false), add_particle(false), delete_particle(false), mouse(), reactor(NULL), now(0) {}

	void add_to_wall_speed(int add) {
		wall_speed += add;
		wall_speed_changed = true;
	}

	void add_to_wall_temp(uint8_t side, double delta) {
		reactor->add_to_wall_gain(side, delta);
	}

	double wall_temp(uint8_t side) const {
		return reactor->wall_gain[side];
	}
};
