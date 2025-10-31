#include <cstdio>
#include <dlfcn.h>
#include <thread>
#include <chrono>
#include <iostream>

#include "extern/plugin.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"

using CreateFn = dr4::DR4Backend *(*)();

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
    std::unique_ptr<dr4::Window> window(backend->CreateWindow());
    window->Open();
    window->SetTitle("plugin demo via dlopen()");

    std::unique_ptr<dr4::Texture> tex(window->CreateTexture());

    tex->SetSize(dr4::Vec2f(200.f, 120.f));

    dr4::Rectangle rect {
        { dr4::Vec2f(10.f, 10.f), dr4::Vec2f(180.f, 100.f) },
        dr4::Color(200, 60, 60, 255),
        4.0f,
        dr4::Color(255, 255, 255, 255)
    };

    tex->Draw(rect);

    window->Clear(dr4::Color(30, 30, 30, 255));
    window->Draw(*tex, dr4::Vec2f(100.f, 80.f));
    window->Display();

    std::this_thread::sleep_for(std::chrono::seconds(2));
    window->Close();
    dlclose(handle);
    return 0;
}
