#include "window.hpp"
#include <SDL3/SDL_render.h>
#include <stdexcept>

DrawWindow::DrawWindow(int width, int height) {
	SDL_SetAppMetadata("Example Draww", "0.1", "com.example.draww");

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
		throw std::runtime_error(SDL_GetError());
	}

	if (!SDL_CreateWindowAndRenderer("example", width, height, 0, &window, &renderer)) {
		SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
		throw std::runtime_error(SDL_GetError());
	}

	SDL_SetRenderDrawColor(renderer, CLR_BG, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);

	SDL_RenderPresent(renderer);
}

DrawWindow::~DrawWindow() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void DrawWindow::clear() {
	SDL_SetRenderDrawColor(renderer, CLR_BG, SDL_ALPHA_OPAQUE);
	SDL_RenderClear(renderer);
}

void DrawWindow::present() {
	SDL_RenderPresent(renderer);
}
