#pragma once
#include "misc/dr4_ifc.hpp"
#include "window.hpp"

class SwuixBackend : public dr4::DR4Backend {
public:
    SwuixBackend() : last_window_(NULL) {}

    const std::string &Name() const {
        static const std::string name = "swuix-sdl3";
        return name;
    }

    dr4::Window *CreateWindow() {
        SwuixWindow *w = new SwuixWindow(dr4::Vec2f(1280.f, 720.f), "swuix");
        last_window_ = w;
        return w;
    }

private:
    SwuixWindow *last_window_;
};

extern "C" dr4::DR4Backend *CreateDR4Backend(void);
