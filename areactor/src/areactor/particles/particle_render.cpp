#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <swuix/window/common.hpp>

#include "particle_manager.hpp"
#include "../reactor.hpp"

void ParticleCircle::draw(SDL_Renderer *renderer, Reactor *reactor) {
	filledCircleRGBA(
		renderer,
		position.x + reactor->frame.x,
		position.y + reactor->frame.y,
		radius,
		CLR_BLUE, SDL_ALPHA_OPAQUE
	);
}

void ParticleSquare::draw(SDL_Renderer *renderer, Reactor *reactor) {
	SDL_FRect d = frect(
		position.x + reactor->frame.x - radius,
		position.y + reactor->frame.y - radius,
		radius * 2, radius * 2);
	SDL_SetRenderDrawColor(renderer, CLR_RASPBERRY, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(renderer, &d);
}
