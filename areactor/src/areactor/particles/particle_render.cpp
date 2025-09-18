#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include "particle_manager.hpp"
#include "../reactor.hpp"
#include "window.hpp"

void ParticleCircle::draw(SDL_Renderer *renderer, Reactor *reactor) {
	filledCircleRGBA(
		renderer,
		position.x + reactor->bbox.x,
		position.y + reactor->bbox.y,
		radius,
		CLR_BLUE, SDL_ALPHA_OPAQUE
	);
}

void ParticleSquare::draw(SDL_Renderer *renderer, Reactor *reactor) {
	SDL_FRect d;
	d.x = position.x + reactor->bbox.x - radius;
	d.y = position.y + reactor->bbox.y - radius;
	d.w = radius * 2;
	d.h = radius * 2;
	SDL_SetRenderDrawColor(renderer, CLR_RASPBERRY, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &d);
}
