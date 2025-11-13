#pragma once
#include <swuix/widgets/button.hpp>

#include "swuix/common.hpp"
#include "swuix/state.hpp"
#include "swuix/window/common.hpp"

class Desktop;

class LaunchObjView : public Action {
    Desktop *root;
    State   *state;

public:
    LaunchObjView(Desktop *r, State *s) : root(r), state(s) {}

    void apply(void *, Widget *);
};

class MenuButton final : public Button {
    void _draw_separator() {
        Rect2f f = frame();
        dr4::Line *l = thickLine(state->window, {f.size.x - 1, 0}, {f.size.x - 1, f.size.y - 2}, {CLR_BORDER}, 1);
        texture->Draw(*l);

        l = thickLine(state->window, {0, f.size.y - 1.5f}, {f.size.x, f.size.y - 1.5f}, {CLR_BORDER}, 2);
        texture->Draw(*l);
    }

public:
    MenuButton(Rect2f f, Widget *p, const char *l, State *s, Action *a)
        : Button(f, p, l, s, a) {}

    MenuButton(Rect2f f, Widget *p, const char *l, State *s, void (*on_click)(void*, Widget*))
        : Button(f, p, l, s, on_click) {}

    MenuButton(Rect2f f, Widget *p, const char *l, State *s)
        : Button(f, p, l, s) {}

    void draw_hover() override {
        Rect2f f = frame();
        Rectangle *r;
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.06);
        r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);

        _draw_separator();
    }

    void draw_press() override {
        Rect2f f = frame();
        Rectangle *r;
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.10);
        r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);

        _draw_separator();
    }

    void draw_idle() override {
        Rect2f f = frame();
        Rectangle *r;
        r = rectFill(state->window, f, {CLR_SURFACE_2});
        texture->Draw(*r);

        _draw_separator();
    }
};

class MenuBar final : public Widget {
    Button *view;

public:
    MenuBar(Rect2f f, Widget *p, State *s, Desktop *r) : Widget(f, p, s) {
        view = new MenuButton({0, 0, 100, f.size.y}, this, "Properties", state, new LaunchObjView(r, state));
        appendChild(view);
    }

    const char *title() const override {
        return "Menu bar";
    }

    void layout() override {
        view->resize({100, texture->GetHeight()});
    }

    void draw() override {
        dr4::Rectangle *r = rectFill(state->window, frame(), {CLR_SURFACE_3});
        texture->Draw(*r);

        float h_off = texture->GetHeight() - 1.5;
        float w = texture->GetWidth();
        dr4::Line *bottom_border = thickLine(state->window, {0, h_off}, {w, h_off}, {CLR_BORDER}, 2);
        texture->Draw(*bottom_border);
    }
};
