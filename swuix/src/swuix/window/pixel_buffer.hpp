#pragma once
#include <SDL3/SDL.h>
#include <stdexcept>

class PixelBuffer {
	SDL_Renderer *renderer;
	const SDL_PixelFormatDetails *details;
	Uint32 grayLUT[256];
public:
	int width, height;
	SDL_Texture *texture;

	PixelBuffer(SDL_Renderer *ren, int w, int h)
			: renderer(ren), width(w), height(h) {
		texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_RGBA32,
			SDL_TEXTUREACCESS_STREAMING,
			width,
			height
		);
		if (!texture) throw std::runtime_error(SDL_GetError());

		details = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32);
		for (int i = 0; i < 256; ++i) {
			grayLUT[i] = SDL_MapRGBA(details, NULL, i, i, i, 255);
		}
	}

	~PixelBuffer() {
		if (texture) SDL_DestroyTexture(texture);
	}

	// if rect is NULL, locks entire texture
	bool lock(const SDL_Rect *rect, void **pixels, int *pitch) {
		return SDL_LockTexture(texture, rect, pixels, pitch);
	}

	void unlock() {
		SDL_UnlockTexture(texture);
	}

	void set_pixel_gray(void *base, int pitch, int x, int y, Uint8 gray) {
		// NOTE: base points to the top-left of the locked rect
		Uint8 *row = (Uint8*)(base) + y * pitch;
		Uint32 *colors = (Uint32*)(row);
		colors[x] = grayLUT[gray];
	}

	void set_pixel_rgba(void *base, int pitch, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a=255) {
		Uint32 px = SDL_MapRGBA(details, NULL, r, g, b, a);
		Uint8 *row = (Uint8*)(base) + y * pitch;
		Uint32 *colors = (Uint32*)(row);
		colors[x] = px;
	}

	void draw() {
		SDL_RenderTexture(renderer, texture, NULL, NULL);
	}
};
