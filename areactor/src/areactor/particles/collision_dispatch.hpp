#pragma once
#include "particle_manager.hpp"

class Reactor;

typedef void (*CollideFn)(Reactor *r, Particle *a, Particle *b, Time now);

void collide_impl(Reactor *r, ParticleCircle *A, ParticleCircle *B, Time now);
void collide_impl(Reactor *r, ParticleCircle *A, ParticleSquare *B, Time now);
void collide_impl(Reactor *r, ParticleSquare *A, ParticleCircle *B, Time now);
void collide_impl(Reactor *r, ParticleSquare *A, ParticleSquare *B, Time now);

template <typename A, typename B>
inline void thunk(Reactor *r, Particle *a, Particle *b, Time now) {
	collide_impl(r, static_cast<A*>(a), static_cast<B*>(b), now);
}

static CollideFn g_collide_tbl[ParticleType::__COUNT][ParticleType::__COUNT] = {
    /* A:Circle */ { &thunk<ParticleCircle, ParticleCircle>, &thunk<ParticleCircle, ParticleSquare> },
    /* A:Square */ { &thunk<ParticleSquare, ParticleCircle>, &thunk<ParticleSquare, ParticleSquare> }
};

inline void collide_dispatch(Reactor *r, Particle *a, Particle *b, Time now) {
	const unsigned ia = a->type_id_;
	const unsigned ib = b->type_id_;
	assert(ia < ParticleType::__COUNT && ib < ParticleType::__COUNT);
	CollideFn fn = g_collide_tbl[ia][ib];
	fn(r, a, b, now);
}
