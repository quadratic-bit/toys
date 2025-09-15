#pragma once
#include <SDL3/SDL_rect.h>

#include "vectors.hpp"

struct Side {
	enum Enum {
		NONE=0,
		LEFT,
		RIGHT,
		BOTTOM,
		TOP
	};
};

struct EventType {
	enum Enum {
		PARTICLE_PARTICLE,
		PARTICLE_WALL,
		CELL_CROSS
	};
};

class Event {
	double time;
	unsigned type;
	unsigned seq;

	int particle_a; // all events
	int particle_b; // PARTICLE_PARTICLE

	int gen_a; // all events
	int gen_b; // PARTICLE_PARTICLE

	int wall_side; // PARTICLE_WALL
	char axis; // CELL_CROSS

	static Event ParticleParticle();
};

class Particle {
public:
	Vector2 position;
	Vector2 velocity;
	double radius;
	double mass;
	int gen;
	bool alive;

	Particle(Vector2 pos, Vector2 vel, double r, double m)
		: position(pos), velocity(vel), radius(r), mass(m), gen(1), alive(true) {}
};

class Reactor {
public:
	SDL_FRect bbox;

	Reactor(SDL_FRect rect) : bbox(rect) {}
};
