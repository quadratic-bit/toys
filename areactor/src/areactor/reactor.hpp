#pragma once
#include <cassert>
#include <cmath>
#include <limits>
#include <typeinfo>
#include <vector>

#include <swuix/widgets/handle.hpp>

#include "linalg/vectors.hpp"
#include "particles/particle_manager.hpp"
#include "particles/collision_dispatch.hpp"
#include "ring_buffer.hpp"

static const double SMALL_RADIUS = 2.0;
static const double TIME_EPS     = 1e-12;  // seconds
static const int    GRID_W       = 16;
static const int    GRID_H       = 9;

static const double kB = 1.0;

struct WallProbe {
	RingBuffer<double> m_vn2_in_last;

	RingBuffer<double> impulse_last;
	RingBuffer<double> dt_last;

	WallProbe() : m_vn2_in_last(128), impulse_last(32), dt_last(32) {}
};

inline double inext(double x) {
	return nextafter(x,  std::numeric_limits<double>::infinity());
}

inline double inprev(double x) {
	return nextafter(x, -std::numeric_limits<double>::infinity());
}

inline double smallest(double x, double y, double z, double w){
	return std::min(x, std::min(y, std::min(z, w)));
}

struct Side {
	enum Enum {
		NONE = 0,
		LEFT,
		RIGHT,
		BOTTOM,
		TOP,
		__COUNT
	};
};

struct Stat {
	size_t n;
	double total_mass;
	double kinetic;      // thermal kinetic energy
	double temperature;  // instantaneous T
	Vec2f   bulk_u;       // mass-weighted mean velocity
	size_t n_circle;
	size_t n_square;

	unsigned right_hits;
	double   right_impulse_sum;
	double   right_pressure;
	double   right_temperature;
};

struct WallSeg {
	Time t0;
	double x0;  // x(t0)
	double v;
	unsigned gen;
	WallSeg() : t0(0), x0(0), v(0), gen(1) {}
};

class Reactor : public HandledWidget {
	ParticleID seq;

	WallSeg right_wall;
	unsigned seg_gen_left;
	unsigned seg_gen_bottom;
	unsigned seg_gen_top;

	WallProbe right_probe;

	int compute_substeps(Time dt) const {
		const std::vector<Slot> &act = particles->active_slots;

		if (act.empty()) return 1;

		double vmax = std::abs(wall_vel(Side::RIGHT));
		double rmin = std::numeric_limits<double>::infinity();

		for (size_t i = 0; i < act.size(); ++i) {
			const Particle* p = particles->items[act[i]];
			if (!p || !p->alive) continue;
			double v = std::sqrt(p->velocity ^ p->velocity);
			if (v > vmax) vmax = v;
			if (p->radius < rmin) rmin = p->radius;
		}

		if (!std::isfinite(rmin)) rmin = SMALL_RADIUS;

		const double cw = particles->grid->cell_w;
		const double ch = particles->grid->cell_h;

		const double max_disp = 0.5 * std::min(rmin, std::min(cw, ch));

		if (vmax <= 1e-12) return 1;

		int N = (int)std::ceil((vmax * dt) / max_disp);

		if (N < 1) N = 1;
		if (N > 256) N = 256;
		return N;
	}

	void integrate_positions(Time t0, Time dt) {
		std::vector<Slot> &act = particles->active_slots;
		for (size_t i = 0; i < act.size(); ++i) {
			Particle* p = particles->items[act[i]];
			if (!p || !p->alive) continue;
			p->position += p->velocity * dt;
			p->last_moved = t0 + dt;
		}
	}

	void handle_walls(Time now) {
		std::vector<Slot> &act = particles->active_slots;
		for (size_t i = 0; i < act.size(); ++i) {
			Particle* p = particles->items[act[i]];
			if (!p || !p->alive) continue;

			bool bounced = false;
			// LEFT
			if (p->position.x < 0.0 + p->radius) {
				p->position.x = inext(0.0 + p->radius);
				p->velocity.x = -wall_gain[Side::LEFT] * p->velocity.x;
				bounced = true;
			}
			// RIGHT (moving)
			{
				const double Xw = wall_pos(Side::RIGHT, now);
				if (p->position.x > Xw - p->radius) {
					p->position.x = inprev(Xw - p->radius);
					const double w = wall_vel(Side::RIGHT);
					const double e = wall_gain[Side::RIGHT];
					const Vec2f n(-1.0, 0.0);
					const double vn_in_rel = w - (p->velocity ^ n);
					if (vn_in_rel > 0.0) {
						this->right_probe.m_vn2_in_last.push(p->mass * vn_in_rel * vn_in_rel);
					}
					const double u = p->velocity.x - w;
					p->velocity.x = w - e * u;
					bounced = true;
				}
			}
			// TOP
			if (p->position.y < 0.0 + p->radius) {
				p->position.y = inext(0.0 + p->radius);
				p->velocity.y = -wall_gain[Side::TOP] * p->velocity.y;
				bounced = true;
			}
			// BOTTOM
			if (p->position.y > frame.h - p->radius) {
				p->position.y = inprev(frame.h - p->radius);
				p->velocity.y = -wall_gain[Side::BOTTOM] * p->velocity.y;
				bounced = true;
			}
			if (bounced) p->gen++;
		}
	}

	void rebuild_buckets_if_needed() {
		std::vector<Slot>& act = particles->active_slots;
		for (size_t i = 0; i < act.size(); ++i) {
			Slot s = act[i];
			Particle* p = particles->items[s];
			if (!p || !p->alive) continue;
			CellHandle now_c = particles->grid->cell_index(p->position);
			if (now_c != particles->cell_of[s]) {
				move_cell(s, now_c);
			}
		}
	}

	bool detect_and_collide_pairs(Time now) {
		// find first overlapping pair and process it
		std::vector<Slot> &act = particles->active_slots;
		for (size_t ia = 0; ia < act.size(); ++ia) {
			Slot sa = act[ia];
			Particle* A = particles->items[sa];
			if (!A || !A->alive) continue;
			Cell c = particles->grid->cell(A->position);
			for (int dy = -1; dy <= 1; ++dy) {
				int cy = c.y + dy; if (cy < 0 || cy >= particles->grid->ny) continue;
				for (int dx = -1; dx <= 1; ++dx) {
					int cx = c.x + dx; if (cx < 0 || cx >= particles->grid->nx) continue;
					Cell cc;
					cc.x = cx;
					cc.y = cy;
					CellHandle ch = particles->grid->cell_handle(cc);
					// NOTE: don't cache size() as bucket can change after collision
					for (size_t k = 0; k < particles->grid->buckets[ch].size(); ++k) {
						Slot sb = particles->grid->buckets[ch][k];
						if (sb == sa) continue;
						Particle *B = (sb < particles->items.size()) ? particles->items[sb] : NULL;
						if (!B || !B->alive) continue;
						if (A->id >= B->id) continue;
						const Vec2f d = B->position - A->position;
						const double rr = (A->radius + B->radius);
						if ((d ^ d) <= rr * rr) {
							collide_dispatch(this, A, B, now);
							// NOTE: must end here, as collision might've changed the bucket
							return true;
						}
					}
				}
			}
		}
		return false;
	}

public:
	ParticleManager *particles;

	Time sim_now;

	double wall_gain[5];

	void add_particles(Time t, size_t n) {
		for (size_t i = 0; i < n; ++i) {
			Vec2f pos = Vec2f::random_rect(frame.w - SMALL_RADIUS * 2, frame.h - SMALL_RADIUS * 2) + Vec2f(SMALL_RADIUS, SMALL_RADIUS);
			Vec2f vel = Vec2f::random_radial(200, 300);
			Particle *new_p = new ParticleCircle(particles->seq++, pos, vel, SMALL_RADIUS, 1, t);
			particles->add(new_p);
		}
	}

	void remove_particle() {
		size_t n = particles->active_slots.size();
		if (n == 0) return;
		particles->remove(particles->items[particles->active_slots[n - 1]]->id);
	}

	void remove_particles(size_t request) {
		size_t n = particles->active_slots.size();
		if (n < request) request = n;
		for (size_t i = 0; i < request; ++i, --n) {
			particles->remove(particles->items[particles->active_slots[n - 1]]->id);
		}
	}

	Reactor(FRect rect, Widget *parent_, size_t n, State *s)
			: Widget(rect, parent_, s), HandledWidget(rect, parent_, s), seq(0), right_probe() {
		particles = new ParticleManager(16, 9, rect.w / (double)GRID_W, rect.h / (double)GRID_H);
		sim_now = 0.0;

		right_wall.t0 = sim_now;
		right_wall.x0 = frame.w;
		right_wall.v = 0.0;
		right_wall.gen = 1;

		for (int i = 0; i < 5; ++i) wall_gain[i] = 1.0;

		seg_gen_left = seg_gen_bottom = seg_gen_top = 1;

		add_particles(sim_now, n);
	}

	void set_wall_gain(uint8_t side, double g) {
		assert(side < Side::__COUNT);
		if (g < 0.0) g = 0.0;
		wall_gain[side] = g;
	}

	void add_to_wall_gain(uint8_t side, double dg) {
		set_wall_gain(side, wall_gain[side] + dg);
	}

	void move_cell(Slot slot, CellHandle new_cell) {
		CellHandle old_cell = particles->cell_of[slot];
		if (old_cell == new_cell) return;
		particles->bucket_erase(old_cell, slot);
		particles->bucket_push(new_cell, slot);
	}

	// TODO: code duplication
	void resolve_wall_overlap_now(Particle *p) const {
		const double xmin = 0.0, xmax = frame.w;
		const double ymin = 0.0, ymax = frame.h;

		bool hitL = false, hitR = false, hitT = false, hitB = false;

		if (p->position.x < xmin + p->radius) { p->position.x = inext (xmin + p->radius); hitL = true; }
		if (p->position.x > xmax - p->radius) { p->position.x = inprev(xmax - p->radius); hitR = true; }
		if (p->position.y < ymin + p->radius) { p->position.y = inext (ymin + p->radius); hitT = true; }
		if (p->position.y > ymax - p->radius) { p->position.y = inprev(ymax - p->radius); hitB = true; }

		if (hitL && p->velocity.x < 0.0) p->velocity.x = -wall_gain[Side::LEFT]   * p->velocity.x;
		if (hitR && p->velocity.x > 0.0) p->velocity.x = -wall_gain[Side::RIGHT]  * p->velocity.x;
		if (hitT && p->velocity.y < 0.0) p->velocity.y = -wall_gain[Side::TOP]    * p->velocity.y;
		if (hitB && p->velocity.y > 0.0) p->velocity.y = -wall_gain[Side::BOTTOM] * p->velocity.y;
	}

	void step_frame(Time dt) {
		const Time start = sim_now;
		const Time end = start + dt;
		(void)end;
		int N = compute_substeps(dt);
		const double h = dt / (double)N;

		for (int i = 0; i < N; ++i) {
			const Time t_sub_start = sim_now;
			const Time t_sub_end   = sim_now + h;
			frame.w = wall_pos(Side::RIGHT, t_sub_start);
			handle_widget()->adjust_width(frame.w);

			integrate_positions(t_sub_start, h);
			handle_walls(t_sub_end);
			rebuild_buckets_if_needed();
			for (int pass = 0; pass < 3; ++pass) {
				bool any = detect_and_collide_pairs(t_sub_end);
				if (!any) break;

				rebuild_buckets_if_needed();
			}
			sim_now = t_sub_end;
		}

		frame.w = wall_pos(Side::RIGHT, sim_now);
		handle_widget()->adjust_width(frame.w);
	}

	Stat tally() const {
		Stat s; s.n = 0; s.total_mass = 0.0; s.kinetic = 0.0; s.temperature = 0.0;
		s.n_circle = 0; s.n_square = 0; s.bulk_u = Vec2f(0.0, 0.0);

		// first pass: mass, momentum
		const std::vector<Slot> &active = particles->active_slots;
		for (size_t k = 0; k < active.size(); ++k) {
			const Particle *p = particles->items[active[k]];
			if (!p->alive) continue;
			if (typeid(*p) == typeid(ParticleCircle)) {
				s.n_circle++;
			}
			if (typeid(*p) == typeid(ParticleSquare)) {
				s.n_square++;
			}
			s.n += 1;
			s.total_mass += p->mass;
			s.bulk_u += p->velocity * p->mass;
		}
		if (s.total_mass > 0.0) s.bulk_u /= s.total_mass;

		// second pass: thermal kinetic energy
		double sum_m_v2 = 0.0;
		for (size_t k = 0; k < active.size(); ++k) {
			Particle const* p = particles->items[active[k]];
			if (!p->alive) continue;
			Vec2f dv = p->velocity - s.bulk_u;
			sum_m_v2 += p->mass * (dv ^ dv);
		}
		s.kinetic = 0.5 * sum_m_v2;

		if (right_probe.m_vn2_in_last.size > 0) {
			const double mean_m_vn2 = right_probe.m_vn2_in_last.mean(
					(int)right_probe.m_vn2_in_last.size);
			s.right_temperature = mean_m_vn2 / (2.0 * kB);
		} else {
			s.right_temperature = 0.0;
		}
		return s;
	}

	double wall_pos(int side, Time tt) const {
		switch (side) {
			case Side::LEFT:   return 0.0;
			case Side::RIGHT:  return right_wall.x0 + right_wall.v * (tt - right_wall.t0);
			default:           return 0.0;
		}
	}

	double wall_vel(int side) const {
		switch (side) {
			case Side::LEFT:   return 0.0;
			case Side::RIGHT:  return right_wall.v;
			default:           return 0.0;
		}
	}

	unsigned wall_seg_gen(int side) const {
		switch (side) {
			case Side::LEFT:   return seg_gen_left;
			case Side::RIGHT:  return right_wall.gen;
			case Side::BOTTOM: return seg_gen_bottom;
			case Side::TOP:    return seg_gen_top;
			default:           return 1u;
		}
	}

	void set_right_wall_velocity(double v_right) {
		begin_right_wall_segment(v_right, sim_now);
	}

	void begin_right_wall_segment(double new_v, Time now) {
		double x_now = wall_pos(Side::RIGHT, now);
		right_wall.x0 = x_now;
		right_wall.t0 = now;
		right_wall.v = new_v;
		++right_wall.gen;
	}

	const char *title() const {
		return "Reactor";
	}

	DispatchResult on_idle    (DispatcherCtx, const IdleEvent    *);
	DispatchResult on_key_down(DispatcherCtx, const KeyDownEvent *);
	DispatchResult on_key_up  (DispatcherCtx, const KeyUpEvent   *);

	void render_body(Window *window, int off_x, int off_y);
};
