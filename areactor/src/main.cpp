#include <SDL3/SDL_events.h>
#include <cassert>
#include <cstdio>

#include <swuix/window/window.hpp>
#include <swuix/manager.hpp>

#include "areactor/state.hpp"
#include "widgets/desktop.hpp"

static const int FPS = 60;

int main() {
    Rect2F bbox = frect(0, 0, 1280, 720);
    Window *window = new Window(bbox.w, bbox.h);
    ReactorState state = ReactorState(window);
    EventManager evmgr(&state, FPS);

    Desktop root(bbox, NULL, FPS, &state);

    for (;;) {
        evmgr.prepare_events();

        while (!evmgr.exhaust_events(&root)) {
            evmgr.dispatch_idle(&root);
        }

        if (state.exit_requested) break;

        evmgr.render(&root);
    }

    delete window;
    return 0;
}
