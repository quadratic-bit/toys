#pragma once
#include <queue>

#include "../common.hpp"

class Reactor;

struct Side {
	enum Enum {
		NONE = 0,
		LEFT,
		RIGHT,
		BOTTOM,
		TOP
	};
};

class Event {
public:
	Time    time;

	explicit Event(Time tt) : time(tt) {}
	virtual ~Event() {}

	virtual void dispatch(Reactor *reactor) = 0;
};

class EventCellCross : public Event {
public:
	ParticleID particle;
	int gen;
	char axis;
	CellHandle cell;

	EventCellCross(Time tt, ParticleID p, int g, char ax, CellHandle c) : Event(tt) {
		particle = p;
		gen = g;
		axis = ax;
		cell = c;
	}

	void dispatch(Reactor *reactor);
};

class EventParticleWall : public Event {
public:
	ParticleID particle;
	int gen;
	int side;
	CellHandle cell;

	EventParticleWall(Time tt, ParticleID p, int g, int s, CellHandle c) : Event(tt) {
		particle = p;
		gen = g;
		side = s;
		cell = c;
	}

	void dispatch(Reactor *reactor);
};

class EventParticleParticle : public Event {
public:
	ParticleID particle_a;
	ParticleID particle_b;
	int gen_a;
	int gen_b;
	CellHandle cell;

	EventParticleParticle(Time tt, ParticleID p_a, ParticleID p_b, int g_a, int g_b, CellHandle c) : Event(tt) {
		particle_a = p_a;
		particle_b = p_b;
		gen_a = g_a;
		gen_b = g_b;
		cell = c;
	}

	void dispatch(Reactor *reactor);
};

struct EventLess {
	bool operator()(const Event *A, const Event *B) const {
		return A->time > B->time;
	}
};

typedef std::priority_queue< Event *, std::vector<Event *>, EventLess > EventQueue;
