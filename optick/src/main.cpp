#include <swuix/manager.hpp>

#include "desktop.hpp"

static const int FPS = 60;

#define MS(s) ((s) * (double)1000)

int main() {
    Rect2F bbox = frect(0, 0, 1280, 720);
    Window *window = new Window(bbox.w, bbox.h);

    State state = State(window);
    EventManager evmgr(&state, FPS);

    { // start of `root` scope

    Desktop root(bbox, NULL, &state);

    for (;;) {
        evmgr.prepareEvents();

        while (!evmgr.exhaustEvents(&root)) {
            evmgr.idle(&root);
        }

        if (state.exit_requested) break;

        evmgr.render(&root);
    }

    } // end of `root` scope

    delete window;
    return 0;
}
