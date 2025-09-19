#pragma once
#include <SDL3/SDL_rect.h>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <limits>
#include <typeinfo>
#include <vector>

#include "linalg/vectors.hpp"
#include "particles/particle_manager.hpp"
#include "particles/collision_dispatch.hpp"
#include "events/event_manager.hpp"

static const double SMALL_RADIUS = 2.0;
static const double TIME_EPS     = 1e-12;  // seconds
static const int    GRID_W       = 16;
static const int    GRID_H       = 9;

inline double eps(Time t) { return TIME_EPS * (1.0 + std::abs(t)); }

// TODO: particles escape chamber
inline double sched_eps(Time t) { return 2.0 * eps(t); }

inline double inext(double x) {
	return nextafter(x,  std::numeric_limits<double>::infinity());
}

inline double inprev(double x) {
	return nextafter(x, -std::numeric_limits<double>::infinity());
}

inline double smallest(double x, double y, double z, double w){
	return std::min(x, std::min(y, std::min(z, w)));
}

struct Stat {
	double total_energy;
	double avg_sqr_velocity;
	size_t n;
	size_t n_circle;
	size_t n_square;
};

struct WallSeg {
	Time t0;
	double x0;  // x(t0)
	double v;
	unsigned gen;
	WallSeg() : t0(0), x0(0), v(0), gen(1) {}
};

class Reactor {
	EventQueue *queue;
	ParticleID seq;

	WallSeg right_wall;
	unsigned seg_gen_left;
	unsigned seg_gen_bottom;
	unsigned seg_gen_top;

	double next_axis_time(double pos, double vel, double cell_size, int c) const {
		const double INF = std::numeric_limits<double>::infinity();
		const double eps = 1e-9;

		if (vel > 0) {
			double boundary = (c + 1) * cell_size;
			return (boundary - pos + eps * cell_size) / vel;
		} else if (vel < 0) {
			double boundary = (c) * cell_size;
			return (boundary - pos - eps * cell_size) / vel;
		} else {
			return INF; // no crossing on the axis
		}
	};

	Vector2 predict_pos(const Particle *p, Time t) const {
		double dt = t - p->last_moved;
		return p->position + p->velocity * dt;
	}

	bool collision_dt(const Particle *A, const Particle *B, Time now, double &out_dt) const {
		Vector2 a = A->position;
		Vector2 b = predict_pos(B, now);

		Vector2 rel_pos = b - a;
		Vector2 rel_vel = B->velocity - A->velocity;

		double rr = A->radius + B->radius;
		double vel2 = rel_vel ^ rel_vel;
		if (vel2 == 0.0) return false;

		double b2 = 2.0 * (rel_pos ^ rel_vel);
		double c2 = (rel_pos ^ rel_pos) - rr * rr;

		if (b2 >= 0.0) return false;

		double disc = b2 * b2 - 4.0 * vel2 * c2;
		if (disc < 0.0) return false;

		double t0 = (-b2 - std::sqrt(disc)) / (2.0 * vel2);
		if (t0 < 0.0) return false;

		out_dt = t0;
		return true;
	}
public:
	ParticleManager *particles;
	SDL_FRect bbox;

	void add_particles(Time t, size_t n) {
		for (size_t i = 0; i < n; ++i) {
			Vector2 pos = Vector2::random_rect(bbox.w - SMALL_RADIUS * 2, bbox.h - SMALL_RADIUS * 2) + Vector2(SMALL_RADIUS, SMALL_RADIUS);
			Vector2 vel = Vector2::random_radial(400, 600);
			Particle *new_p = new ParticleCircle(particles->seq++, pos, vel, SMALL_RADIUS, 1, t);
			particles->add(new_p);
			reschedule_all_for(new_p->id, t);
		}
	}

	void remove_particle() {
		size_t n = particles->active_slots.size();
		if (n == 0) return;
		particles->remove(particles->items[particles->active_slots[n - 1]]->id);
	}

	Reactor(SDL_FRect rect, Time t, size_t n) : seq(0), bbox(rect) {
		queue = new EventQueue();
		particles = new ParticleManager(16, 9, rect.w / (double)GRID_W, rect.h / (double)GRID_H);

		right_wall.t0 = t;
		right_wall.x0 = bbox.w;
		right_wall.v = 0.0;
		right_wall.gen = 1;

		seg_gen_left = seg_gen_bottom = seg_gen_top = 1;

		add_particles(t, n);
	}

	void schedule_cell_cross(ParticleID pid, Time t) {
		Slot slot = particles->slot_of_id[pid];
		Particle *p = particles->items[slot];
		Cell cell = particles->grid->cell(p->position);

		double tx = next_axis_time(p->position.x, p->velocity.x, particles->grid->cell_w, cell.x);
		double ty = next_axis_time(p->position.y, p->velocity.y, particles->grid->cell_h, cell.y);

		double dtnext = std::min(tx, ty);
		if (!std::isfinite(dtnext) || dtnext <= 0) return;
		double min_dt = sched_eps(t);
		if (dtnext < min_dt) dtnext = min_dt;

		char axis = (tx < ty) ? 'X' : 'Y';
		EventCellCross *e = new EventCellCross(t + dtnext, p->id, p->gen, axis, particles->grid->cell_handle(cell));
		queue->push(e);
	}

	double dt_to(double pos, double vel, double target) const {
		if (vel == 0.0) return std::numeric_limits<double>::infinity();
		return (target - pos) / vel;
	}

	void schedule_wall_collision(ParticleID pid, Time t) {
		if (!particles->slot_of_id.count(pid)) return;

		Slot slot = particles->slot_of_id[pid];
		Particle *p = particles->items[slot];

		const Vector2 x = predict_pos(p, t);
		const Vector2 v = p->velocity;

		double best_dt = std::numeric_limits<double>::infinity();
		int best_side = Side::NONE;
		unsigned best_seg_gen = 0;

		if (p->velocity.x < 0.0) {
			double gap = (0.0 + p->radius) - x.x;
			double dt  = gap / v.x;
			if (dt >= -1e-12 && dt < best_dt) {
				best_dt = std::max(0.0, dt);
				best_side = Side::LEFT;
				best_seg_gen = wall_seg_gen(Side::LEFT);
			}
		}

		//if (p->velocity.x > 0.0) {
		{
			const int side = Side::RIGHT;
			const double Xw = wall_pos(side, t);
			const double w  = wall_vel(side);
			const double face = Xw - p->radius;
			const double gap  = face - x.x;
			const double rel  = v.x - w;

			if (gap <= 0.0) {
				if (0.0 < best_dt) {
					best_dt = 0.0;
					best_side = side;
					best_seg_gen = wall_seg_gen(side);
				}
			} else if (rel > 0.0) {
				double dt = gap / rel;
				if (dt >= -1e-12 && dt < best_dt) {
					best_dt = std::max(0.0, dt);
					best_side = side;
					best_seg_gen = wall_seg_gen(side);
				}
			}
		}
		if (p->velocity.y < 0.0) {
			double gap = (0.0 + p->radius) - x.y;
			double dt = gap / v.y;
			if (dt >= -1e-12 && dt < best_dt) {
				best_dt = std::max(0.0, dt);
				best_side = Side::TOP;
				best_seg_gen = wall_seg_gen(Side::TOP);
			}
		}
		if (p->velocity.y > 0.0) {
			double gap = (bbox.h - p->radius) - x.y;
			double dt  = gap / v.y;
			if (dt >= -1e-12 && dt < best_dt) {
				best_dt = std::max(0.0, dt);
				best_side = Side::BOTTOM;
				best_seg_gen = wall_seg_gen(Side::BOTTOM);
			}
		}

		if (!std::isfinite(best_dt)) return;

		Cell c = particles->grid->cell(x);

		EventParticleWall *e = new EventParticleWall(t + best_dt, p->id, p->gen, best_side, best_seg_gen, particles->grid->cell_handle(c));
		queue->push(e);
	}

	void schedule_particle_collisions(ParticleID pid, Time t) {
		const Slot slot_a = particles->slot_of_id[pid];
		const Particle *A = particles->items[slot_a];
		if (!A->alive) return;

		// 3x3 neighborhood
		Cell c = particles->grid->cell(A->position);
		for (int dy = -1; dy <= 1; ++dy) {
			int cy = c.y + dy;
			if (cy < 0 || cy >= particles->grid->ny) continue;
			for (int dx = -1; dx <= 1; ++dx) {
				int cx = c.x + dx;
				if (cx < 0 || cx >= particles->grid->nx) continue;
				Cell other_c;
				other_c.x = cx;
				other_c.y = cy;
				CellHandle ch = particles->grid->cell_handle(other_c);
				std::vector<Slot> &bucket = particles->grid->buckets[ch];

				for (size_t i = 0; i < bucket.size(); ++i) {
					Slot slot_b = bucket[i];
					if (slot_b == slot_a) continue;
					const Particle *B = particles->items[slot_b];
					if (!B->alive) continue;

					//if (A->id >= B->id) continue;

					double dt_col;
					if (!collision_dt(A, B, t, dt_col)) continue;

					EventParticleParticle *e = new EventParticleParticle(t + dt_col, A->id, B->id, A->gen, B->gen, ch);
					queue->push(e);
				}
			}
		}
	}

	void bounce_off_wall(Particle *p, int side, Time now) const {
		switch (side) {
		case Side::LEFT: {
			double X = 0.0;
			double target = X + p->radius;
			p->position.x = inext(target);
			p->velocity.x = -p->velocity.x;
		} break;
		case Side::RIGHT: {
			double X = wall_pos(Side::RIGHT, now);
			double w = wall_vel(Side::RIGHT);
			double target = X - p->radius;
			p->position.x = inprev(target);
			p->velocity.x = 2.0 * w - p->velocity.x;
		} break;
		case Side::TOP: {
			double Y = 0.0;
			double target = Y + p->radius;
			p->position.y = inext(target);
			p->velocity.y = -p->velocity.y;
		} break;
		case Side::BOTTOM: {
			double Y = bbox.h;
			double target = Y - p->radius;
			p->position.y = inprev(target);
			p->velocity.y = -p->velocity.y;
		} break;
		default: break;
		}
	}

	void advance_particle_to(Particle *p, Time t) {
		// TODO: keep (x_last, v_last) to avoid floating drift
		double dt = t - p->last_moved;
		if (dt <= 0) return;
		p->position += p->velocity * dt;
		p->last_moved = t;
	}

	void move_cell(Slot slot, CellHandle new_cell) {
		CellHandle old_cell = particles->cell_of[slot];
		if (old_cell == new_cell) return;
		particles->bucket_erase(old_cell, slot);
		particles->bucket_push(new_cell, slot);
	}

	// TODO: code duplication
	void resolve_wall_overlap_now(Particle *p) const {
		const double xmin = 0.0, xmax = bbox.w;
		const double ymin = 0.0, ymax = bbox.h;

		bool hitL = false, hitR = false, hitT = false, hitB = false;

		if (p->position.x < xmin + p->radius) { p->position.x = inext (xmin + p->radius); hitL = true; }
		if (p->position.x > xmax - p->radius) { p->position.x = inprev(xmax - p->radius); hitR = true; }
		if (p->position.y < ymin + p->radius) { p->position.y = inext (ymin + p->radius); hitT = true; }
		if (p->position.y > ymax - p->radius) { p->position.y = inprev(ymax - p->radius); hitB = true; }

		if (hitL && p->velocity.x < 0.0) p->velocity.x = -p->velocity.x;
		if (hitR && p->velocity.x > 0.0) p->velocity.x = -p->velocity.x;
		if (hitT && p->velocity.y < 0.0) p->velocity.y = -p->velocity.y;
		if (hitB && p->velocity.y > 0.0) p->velocity.y = -p->velocity.y;
	}

	void reschedule_all_for(ParticleID pid, Time now) {
		schedule_cell_cross(pid, now);
		schedule_wall_collision(pid, now);
		schedule_particle_collisions(pid, now);
	}

	void step_frame(Time start, Time dt) {
		const Time end = start + dt;

		while (!queue->empty()) {
			Event *e = queue->top();

			if (e->time > end) break;

			queue->pop();

			e->dispatch(this);
			delete e;
		}
		std::vector<Slot> &active_slots = particles->active_slots;
		for (size_t i = 0; i < active_slots.size(); ++i) {
			Slot slot = active_slots[i];
			Particle *p = particles->items[slot];
			advance_particle_to(p, end);
			CellHandle c_now = particles->grid->cell_index(p->position);
			if (c_now != particles->cell_of[slot]) {
				move_cell(slot, c_now);
			}
		}
		bbox.w = wall_pos(Side::RIGHT, end);
	}

	Stat tally() const {
		Stat stat = {0, 0, 0, 0, 0};
		std::vector<Slot> &active_slots = particles->active_slots;
		for (size_t i = 0; i < active_slots.size(); ++i) {
			Slot slot = active_slots[i];
			Particle *p = particles->items[slot];
			if (typeid(*p) == typeid(ParticleCircle)) {
				stat.n_circle++;
			}
			if (typeid(*p) == typeid(ParticleSquare)) {
				stat.n_square++;
			}
			stat.total_energy += p->mass * (p->velocity ^ p->velocity) / 2;
			stat.avg_sqr_velocity += (p->velocity ^ p->velocity);
		}
		stat.n = active_slots.size();
		stat.avg_sqr_velocity = stat.n > 0 ? stat.avg_sqr_velocity / (double)stat.n : 0;
		return stat;
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

	void set_right_wall_velocity(Time now, double v_right) {
		queue->push(new EventWallSegChange(now, Side::RIGHT, v_right));
	}

	void begin_right_wall_segment(double new_v, Time now) {
		double x_now = wall_pos(Side::RIGHT, now);
		right_wall.x0 = x_now;
		right_wall.t0 = now;
		right_wall.v = new_v;
		++right_wall.gen;
	}
};
