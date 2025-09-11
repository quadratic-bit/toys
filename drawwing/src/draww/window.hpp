#pragma once
#include <SDL3/SDL.h>
#include <stdexcept>
#include <vector>

#include "axes.hpp"
#include "linalg.hpp"
#include "pixel_buffer.hpp"
#include "scene.hpp"

class DrawWindow {
	SDL_Window *window;
	SDL_Renderer *renderer;
	PixelBuffer *pb;
public:
	std::vector<Scene*> scenes;

	DrawWindow(int width, int height) {
		SDL_SetAppMetadata("Example Draww", "0.1", "com.example.draww");

		if (!SDL_Init(SDL_INIT_VIDEO)) {
			SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		if (!SDL_CreateWindowAndRenderer(
				"example", width, height, SDL_WindowFlags(0),
				&window, &renderer
		)) {
			SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
			throw std::runtime_error(SDL_GetError());
		}

		pb = new PixelBuffer(renderer, width, height);
		scenes = std::vector<Scene*>();

		SDL_SetRenderDrawColor(renderer, CLR_BG, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}

	~DrawWindow() {
		delete pb;
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	Scene *add_scene(CoordinateSystem *cs) {
		Scene *scene = new Scene(cs, renderer, pb);
		scenes.push_back(scene);
		return scene;
	}

	void clear() {
		SDL_SetRenderDrawColor(renderer, CLR_BG, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}

	void present() {
		SDL_RenderPresent(renderer);
	}
};
