#include <SDL3/SDL_events.h>
#include <cassert>
#include <cstdio>

#include <swuix/window/window.hpp>
#include <swuix/manager.hpp>

#include "areactor/state.hpp"
#include "widgets/desktop.hpp"

static const int FPS = 60;

int main() {
	ReactorState state = ReactorState();
	EventManager evmgr(&state, FPS);

	Desktop root(frect(0, 0, 1280, 720), NULL, FPS, &state);
	Window *window = new Window(root.frame.w, root.frame.h);

	for (;;) {
		evmgr.prepare_events();

		while (!evmgr.exhaust_events(&root)) {
			evmgr.dispatch_idle(&root);
		}

		if (state.exit_requested) break;

		root.render(window, 0, 0);
	}

	delete window;
	return 0;
}
