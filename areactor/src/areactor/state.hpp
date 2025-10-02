#pragma once
#include <swuix/widget.hpp>
#include <swuix/state.hpp>

#include "reactor.hpp"

struct ReactorState : public State {
	int wall_speed;
	bool wall_speed_changed;

	bool add_particle;
	bool delete_particle;

	Reactor *reactor;

	bool running;

	ReactorState() : State(), wall_speed(0), wall_speed_changed(false), add_particle(false),
			delete_particle(false), reactor(NULL), running(true) {}

	ReactorState(Time now_) : State(now_), wall_speed(0), wall_speed_changed(false), add_particle(false),
			delete_particle(false), reactor(NULL), running(true) {}

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
