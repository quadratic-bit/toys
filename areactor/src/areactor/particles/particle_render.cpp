#include <swuix/window/common.hpp>
#include <swuix/window/window.hpp>

#include "particle_manager.hpp"
#include "../reactor.hpp"

void ParticleCircle::draw(Window *window, Reactor *reactor) {
	window->draw_filled_circle_rgb(
		position.x + reactor->frame.x,
		position.y + reactor->frame.y,
		radius,
		CLR_BLUE
	);
}

void ParticleSquare::draw(Window *window, Reactor *reactor) {
	window->draw_filled_rect_rgb(
		frect(position.x + reactor->frame.x - radius,
			position.y + reactor->frame.y - radius,
			radius * 2,
			radius * 2),
		CLR_RASPBERRY
	);
}
