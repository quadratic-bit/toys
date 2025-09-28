#pragma once
#include <SDL3/SDL_render.h>
#include <cstdio>
#include <tr1/unordered_map>
#include <vector>

#include "../linalg/vectors.hpp"
#include "../common.hpp"

class Reactor;

static const int BASE_GEN = 1;

struct ParticleType {
	enum Enum {
		CIRCLE = 0,
		SQUARE = 1,
		__COUNT = 2
	};
};

class Particle {
	const unsigned type_id_;
public:
	ParticleID id;

	Vec2 position;
	Vec2 velocity;
	double  radius;

	double  mass;
	int     gen;
	bool    alive;
	Time    last_moved;

	explicit Particle(ParticleID id_, unsigned type_id)
		: type_id_(type_id), id(id_), mass(1.0), gen(BASE_GEN), alive(true) {}
	virtual ~Particle() {}

	virtual void draw(SDL_Renderer *renderer, Reactor *reactor) = 0;

	friend void collide_dispatch(Reactor *r, Particle *a, Particle *b, Time now);
};

class ParticleCircle : public Particle {
	enum { kType = ParticleType::CIRCLE };
public:
	ParticleCircle(ParticleID id_, Vec2 p, Vec2 v, double r, double m, Time t)
			: Particle(id_, kType) {
		position = p;
		velocity = v;
		radius = r;
		mass = m;
		last_moved = t;
	}

	void draw(SDL_Renderer *renderer, Reactor *reactor);
};

class ParticleSquare : public Particle {
	enum { kType = ParticleType::SQUARE };
public:
	ParticleSquare(ParticleID id_, Vec2 p, Vec2 v, double r, double m, Time t)
			: Particle(id_, kType) {
		position = p;
		velocity = v;
		radius = r;
		mass = m;
		last_moved = t;
	}

	void draw(SDL_Renderer *renderer, Reactor *reactor);
};

inline int clamp(int value, int min, int max) {
	return std::min(std::max(value, min), max);
}

struct Cell {
	int x;
	int y;
};

struct Grid {
	int nx, ny;
	double cell_w, cell_h;
	std::vector< std::vector<Slot> > buckets;  // (dense) vector of slots in a cell (cx, cy)

	Grid(int nx_, int ny_, double cw_, double ch_)
		: nx(nx_), ny(ny_), cell_w(cw_), cell_h(ch_), buckets(size_t(nx_ * ny_)) {}

	Cell cell(const Vec2 &pos) const {
		Cell c;
		c.x = clamp((int)std::floor(pos.x / cell_w), 0, nx - 1);
		c.y = clamp((int)std::floor(pos.y / cell_h), 0, ny - 1);
		return c;
	}

	CellHandle cell_handle(Cell c) const {
		return c.x + c.y * nx;
	}

	CellHandle cell_index(const Vec2 &pos) const {
		return cell_handle(cell(pos));
	}
};

class ParticleManager {
public:
	// (dense) vector of slots
	std::vector<Particle *> items;

	// slot metadata for different types of access
	std::vector<size_t> idx_in_bucket;  // slot -> index inside cell bucket (index into Grid::buckets[cell])
	std::vector<CellHandle> cell_of;    // slot -> current cell index (index into Grid::buckets)
	std::vector<size_t> pos_in_active;  // slot -> index into Particles::active_slots
	std::vector<Slot> active_slots;     // (dense) vector of live slots
	std::vector<Slot> freelist;         // free slot stack

	// lookups by ParticleID
	std::tr1::unordered_map<ParticleID, Slot> slot_of_id;  // index into Particle::items (i.e. slot)
	ParticleID seq;

	Grid *grid;

	ParticleManager(int nx_, int ny_, double cw_, double ch_) {
		grid = new Grid(nx_, ny_, cw_, ch_);
	}

	// Add particle items[slot] to a bucket at cell
	void bucket_push(CellHandle cell, Slot slot) {
		std::vector<Slot> &bucket = grid->buckets[cell];
		idx_in_bucket[slot] = bucket.size();
		bucket.push_back(slot);
		cell_of[slot] = cell;
	}

	// Remove particle items[slot] from a bucket at cell
	void bucket_erase(CellHandle cell, Slot slot) {
		std::vector<Slot> &bucket = grid->buckets[cell];
		size_t i = idx_in_bucket[slot];
		size_t j = bucket.size() - 1;
		// make bucket dense
		if (i != j) {
			Slot moved_slot = bucket[j];
			bucket[i] = moved_slot;
			idx_in_bucket[moved_slot] = i;
		}
		bucket.pop_back();
	}

	// Register a new particle
	void active_push(Slot slot) {
		pos_in_active[slot] = active_slots.size();
		active_slots.push_back(slot);
	}

	// Erase a particle from registry
	void active_erase(Slot slot) {
		delete items[slot];
		size_t i = pos_in_active[slot];
		size_t j = active_slots.size() - 1;
		if (i != j) {
			Slot moved_slot = active_slots[j];
			active_slots[i] = moved_slot;
			pos_in_active[moved_slot] = i;
		}
		active_slots.pop_back();
	}

	Slot add(Particle *p) {
		Slot slot;
		if (!freelist.empty()) {  // there is unused space
			slot = freelist.back();  // copy
			freelist.pop_back();
			items[slot] = p;
		} else {  // items vector (with slots) is full
			slot = items.size();
			items.push_back(p);
			idx_in_bucket.push_back(0);  // will be updated before the end of the function
			cell_of.push_back(0);
			pos_in_active.push_back(0);
		}
		slot_of_id[p->id] = slot;
		active_push(slot);
		CellHandle cell = grid->cell_index(p->position);
		bucket_push(cell, slot);
		return slot;
	}

	void remove(ParticleID pid) {
		Slot slot = slot_of_id[pid];
		CellHandle cell = cell_of[slot];
		bucket_erase(cell, slot);

		if (items[slot]) items[slot]->alive = false;

		active_erase(slot);
		slot_of_id.erase(pid);

		items[slot] = NULL;
		freelist.push_back(slot);
	}
};
