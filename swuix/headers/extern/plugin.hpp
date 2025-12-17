#pragma once
#include <cum/ifc/dr4.hpp>

#include "./window.hpp"

class SwuixBackend : public cum::DR4BackendPlugin {
    SwuixWindow *last_window_{nullptr};

public:
    SwuixBackend() = default;
    ~SwuixBackend() override = default;

    // ----- cum::Plugin -----
    std::string_view GetIdentifier() const override {
        return "dr4.backend.swuix-sdl3";
    }

    std::string_view GetName() const override {
        return "Swuix SDL3 DR4 backend";
    }

    std::string_view GetDescription() const override {
        return "DR4 backend implemented using SDL3 + SDL3_ttf + SDL3_gfx";
    }

    std::vector<std::string_view> GetDependencies() const override {
        return {};
    }

    std::vector<std::string_view> GetConflicts() const override {
        return {};
    }

    void AfterLoad() override {}

    // ----- cum::DR4BackendPlugin -----
    dr4::Window *CreateWindow() override {
        auto *w = new SwuixWindow(dr4::Vec2f(1280.f, 720.f), "swuix");
        last_window_ = w;
        return w;
    }
};
