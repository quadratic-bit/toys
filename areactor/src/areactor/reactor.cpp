#include <sstream>

#include "reactor.hpp"
#include "state.hpp"
#include <swuix/window/window.hpp>

inline void draw_bounding_line(SDL_Renderer *renderer, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, unsigned thick, double e) {
	double t = std::max(0.0, std::min(1.0, (e - 1.0) * 0.8 + 0.5)); // center 1.0 at 0.5

	static const uint8_t BLUE[3]      = { CLR_BLUE };
	static const uint8_t NIGHT[3]     = { CLR_NIGHT };
	static const uint8_t RASPBERRY[3] = { CLR_RASPBERRY };

	uint8_t r, g, b;
	if (t <= 0.5) {
		double u = (t / 0.5); // 0..1 between BLUE -> NIGHT
		r = (uint8_t)(BLUE[0] + (NIGHT[0] - BLUE[0]) * u + 0.5);
		g = (uint8_t)(BLUE[1] + (NIGHT[1] - BLUE[1]) * u + 0.5);
		b = (uint8_t)(BLUE[2] + (NIGHT[2] - BLUE[2]) * u + 0.5);
	} else {
		double u = ((t - 0.5) / 0.5); // 0..1 between NIGHT -> RASPBERRY
		r = (uint8_t)(NIGHT[0] + (RASPBERRY[0] - NIGHT[0]) * u + 0.5);
		g = (uint8_t)(NIGHT[1] + (RASPBERRY[1] - NIGHT[1]) * u + 0.5);
		b = (uint8_t)(NIGHT[2] + (RASPBERRY[2] - NIGHT[2]) * u + 0.5);
	}

	thickLineRGBA(renderer, x1, y1, x2, y2, thick, r, g, b, SDL_ALPHA_OPAQUE);
}

inline void draw_particles(SDL_Renderer *renderer, Reactor *reactor) {
	std::vector<Slot> &active_slots = reactor->particles->active_slots;
	for (size_t i = 0; i < active_slots.size(); ++i) {
		Slot slot = active_slots[i];
		Particle *p = reactor->particles->items[slot];
		p->draw(renderer, reactor);
	}
}

void Reactor::render_body(Window *window, int off_x, int off_y) {
	// bg
	window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);

	Stat stats = tally();

	// outline
	Sint16 x = frame.x, y = frame.y;
	Sint16 w = frame.w, h = frame.h;
	draw_bounding_line(window->renderer, x, y, x + w, y, 2, this->wall_gain[Side::TOP]);
	draw_bounding_line(window->renderer, x, y, x, y + h, 2, this->wall_gain[Side::LEFT]);
	draw_bounding_line(window->renderer, x + w, y, x + w, y + h, 2, this->wall_gain[Side::RIGHT]);
	draw_bounding_line(window->renderer, x, y + h, x + w, y + h, 2, this->wall_gain[Side::BOTTOM]);

	// particles
	draw_particles(window->renderer, this);

	std::ostringstream oss;

	oss << stats.n_circle;
	std::string n_circles = oss.str();
	oss.str("");

	n_circles.append(" circle");
	window->text(n_circles.c_str(), frame.x, frame.y + frame.h);

	oss << stats.n_square;
	std::string n_square = oss.str();

	n_square.append(" squares");
	window->text(n_square.c_str(), frame.x, frame.y + frame.h + 20);
}


DispatchResult Reactor::on_idle(DispatcherCtx ctx, const IdleEvent *e) {
	(void)ctx;
	ReactorState *rst = (ReactorState*)state;
	if (rst->add_particle) add_particles(sim_now, 2);
	if (rst->delete_particle) remove_particle();

	if (rst->wall_speed_changed) {
		set_right_wall_velocity(rst->wall_speed);
		rst->wall_speed_changed = false;
	}
	step_frame(e->dt_s);
	return PROPAGATE;
}
