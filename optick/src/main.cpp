#include <cstdio>
#include <memory>
#include <swuix/manager.hpp>
#include <misc/dr4_ifc.hpp>
#include <dlfcn.h>

#include "desktop.hpp"
#include "state.hpp"

static const int FPS = 60;

using CreateFn = dr4::DR4Backend *(*)();

#define MS(s) ((s) * (double)1000)

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " /path/to/libdr4backend.so\n";
        return 2;
    }
    const char *plugin_path = argv[1];
    void *handle = dlopen(plugin_path, RTLD_NOW);
    if (!handle) {
        std::cerr << "dlopen failed: " << dlerror() << "\n";
        return 1;
    }
    dlerror();

    CreateFn create = reinterpret_cast<CreateFn>(dlsym(handle, "CreateDR4Backend"));
    const char *err = dlerror();
    if (err || !create) {
        std::cerr << "dlsym(CreateDR4Backend) failed: " << (err ? err : "null") << "\n";
        dlclose(handle);
        return 1;
    }

    std::unique_ptr<dr4::DR4Backend> backend(create());
    dr4::Window *window(backend->CreateWindow());

    Rect2f bbox {0, 0, 1280, 720};
    window->Open();
    window->SetSize(bbox.size);

    OptickState state = OptickState(window);
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

    window->Close();
    delete window;
    return 0;
}
