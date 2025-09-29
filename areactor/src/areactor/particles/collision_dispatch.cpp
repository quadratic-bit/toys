#include "collision_dispatch.hpp"
#include "../reactor.hpp"

void collide_impl(Reactor *r, ParticleCircle *A, ParticleCircle *B, Time now) {
	const double mass_a = A->mass;
	const double mass_b = B->mass;
	const Vec2f p_tot = A->velocity * mass_a + B->velocity * mass_b;

	const double mass_comb = mass_a + mass_b;
	const Vec2f vel_comb = p_tot / mass_comb;
	const double r_comb = std::sqrt(A->radius * A->radius + B->radius * B->radius);

	Particle *merged_p = new ParticleSquare(
			r->particles->seq++,
			(A->position * mass_a + B->position * mass_b) / mass_comb,
			vel_comb,
			r_comb,
			mass_comb,
			now);

	r->resolve_wall_overlap_now(merged_p);

	r->particles->remove(A->id);
	r->particles->remove(B->id);
	r->particles->add(merged_p);
}

void collide_impl(Reactor *r, ParticleSquare *A, ParticleCircle *B, Time now) {
	const double mass_a = A->mass;
	const double mass_b = B->mass;
	const Vec2f p_tot = A->velocity * mass_a + B->velocity * mass_b;

	const double mass_comb = mass_a + mass_b;
	const Vec2f vel_comb = p_tot / mass_comb;
	const double r_comb = std::sqrt(A->radius * A->radius + B->radius * B->radius);

	A->position = (A->position * mass_a + B->position * mass_b) / mass_comb;
	A->velocity = vel_comb;
	A->radius = r_comb;
	A->mass = mass_comb;
	A->last_moved = now;
	A->gen++;

	r->resolve_wall_overlap_now(A);

	r->particles->remove(B->id);
}

void collide_impl(Reactor *r, ParticleCircle *A, ParticleSquare *B, Time now) {
	collide_impl(r, B, A, now);
}

void collide_impl(Reactor *r, ParticleSquare *A, ParticleSquare *B, Time now) {
	const double mass_a = A->mass;
	const double mass_b = B->mass;
	const Vec2f p_tot = A->velocity * mass_a + B->velocity * mass_b;

	const int N = int(round(mass_a));
	const int M = int(round(mass_b));
	const int K = std::max(2, N + M);
	const Vec2f center = (A->position + B->position) * 0.5;

	// safe ring radius so fragments don't overlap (and try to keep inside bbox)
	const double r_frag = SMALL_RADIUS;  // default radius
	const double need_r = (r_frag / std::sin(M_PI / K));  // tangent neighbors
	const double safe_r = std::max(0.0, smallest(center.x - r_frag,
				r->frame.w - center.x - r_frag,
				center.y - r_frag,
				r->frame.h - center.y - r_frag));
	const double R = std::min(need_r, safe_r * 0.9);

	const Vec2f u = p_tot / double(K);
	const double burst = 120.0; // outward speed

	r->particles->remove(A->id);
	r->particles->remove(B->id);

	for (int i = 0; i < K; ++i) {
		double ang = 2.0 * M_PI * (double(i) / double(K));
		Vec2f dir(std::cos(ang), std::sin(ang));
		Vec2f pos = center + dir * R;
		Vec2f vel = u + dir * burst;

		Particle *new_p = new ParticleCircle(r->particles->seq++, pos, vel, r_frag, 1.0, now);
		r->particles->add(new_p);
	}
}
