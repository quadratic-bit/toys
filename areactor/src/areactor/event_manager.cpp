#include "collision_dispatch.hpp"
#include "event_manager.hpp"
#include "reactor.hpp"

void EventCellCross::dispatch(Reactor *r) {
	if (!r->particles->slot_of_id.count(particle)) return;

	Slot slot = r->particles->slot_of_id[particle];
	Particle *p = r->particles->items[slot];

	if (p->gen != gen) return;

	r->advance_particle_to(p, time);

	const CellHandle new_cell = r->particles->grid->cell_index(p->position);
	r->move_cell(slot, new_cell);
	p->gen++;
	r->reschedule_all_for(p->id, time);
}

void EventParticleWall::dispatch(Reactor *r) {
	if (!r->particles->slot_of_id.count(particle)) return;

	Slot slot = r->particles->slot_of_id[particle];
	Particle *p = r->particles->items[slot];

	if (p->gen != gen) return;

	r->advance_particle_to(p, time);

	r->bounce_off_wall(p, side);
	p->gen++;
	r->reschedule_all_for(p->id, time);
}

void EventParticleParticle::dispatch(Reactor *r) {
	if (!r->particles->slot_of_id.count(particle_a)) return;
	if (!r->particles->slot_of_id.count(particle_b)) return;

	Slot sa = r->particles->slot_of_id[particle_a];
	Slot sb = r->particles->slot_of_id[particle_b];
	Particle *A = r->particles->items[sa];
	Particle *B = r->particles->items[sb];

	if (!A->alive || !B->alive) return;
	if (A->gen != gen_a || B->gen != gen_b) return;

	r->advance_particle_to(A, time);
	r->advance_particle_to(B, time);
	r->move_cell(sa, r->particles->grid->cell_index(A->position));
	r->move_cell(sb, r->particles->grid->cell_index(B->position));

	collide_dispatch(r, A, B, time);
}
