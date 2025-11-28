#include <cstdio>
#include <swuix/manager.hpp>
#include <dlfcn.h>

#include <cum/manager.hpp>
#include <cum/ifc/dr4.hpp>
#include <cum/ifc/pp.hpp>

#include "desktop.hpp"
#include "state.hpp"

static const int FPS = 60;

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " /path/to/libdr4backend.so\n";
        return 2;
    }

    cum::Manager mgr;

    cum::Plugin *backendBase = mgr.LoadFromFile(argv[1]);
    auto *backend = dynamic_cast<cum::DR4BackendPlugin*>(backendBase);
    if (!backend) {
        std::cerr << "Plugin " << argv[1] << " is not a DR4 backend\n";
        return 1;
    }

    for (int i = 2; i < argc; ++i) {
        mgr.LoadFromFile(argv[i]);
    }

    mgr.TriggerAfterLoad();

    dr4::Window *window = backend->CreateWindow();

    Rect2f bbox {0, 0, 1280, 720};
    window->Open();
    window->SetSize(bbox.size);

    dr4::Font *appfont = window->CreateFont();
    appfont->LoadFromFile("/usr/share/fonts/TTF/CaskaydiaCoveNerdFontMono-Regular.ttf");
    OptickState state = OptickState(window, appfont);
    EventManager evmgr(&state, FPS);

    { // start of `root` scope

    Desktop root(bbox, NULL, &state, &mgr);

    for (;;) {
        evmgr.prepareEvents();

        while (!evmgr.exhaustEvents(&root)) {
            evmgr.idle(&root);
        }

        if (state.exit_requested) break;

        evmgr.render(&root);
    }

    } // end of `root` scope

    window->Close();
    delete window;
    return 0;
}
