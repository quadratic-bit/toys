#pragma once
#include <swuix/widgets/button.hpp>
#include <swuix/common.hpp>
#include <swuix/state.hpp>
#include <swuix/window/common.hpp>

#include "plugins_dropdown.hpp"
#include "theme_picker_action.hpp"

class Desktop;
class Renderer;

class ScreenshotAction final : public Action {
    Desktop *root_;
public:
    ScreenshotAction(Desktop *r) : root_(r) {}
    void apply(void *, Widget *) override;
};

class LaunchObjView : public Action {
    Desktop *root;
    State   *state;

public:
    LaunchObjView(Desktop *r, State *s) : root(r), state(s) {}

    void apply(void *, Widget *);
};

class ToggleRender : public Action {
    Desktop *root;

public:
    ToggleRender(Desktop *r) : root(r) {}

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
    Button *toggle;
    Button *plugins;
    Button *shot;
    Button *themeBtn;

public:
    MenuBar(Rect2f f, Widget *p, State *s, Desktop *r) : Widget(f, p, s) {
        view = new MenuButton({0, 0, 100, f.size.y}, this, "Properties", state, new LaunchObjView(r, state));
        toggle = new MenuButton({100, 0, 70, f.size.y}, this, "Toggle", state, new ToggleRender(r));
        shot = new MenuButton({170, 0, 90, f.size.y}, this, "Shot",
                              state, new ScreenshotAction(r));
        plugins = new MenuButton({260, 0, 80, f.size.y}, this, "Plugins",
                                 state, new TogglePluginsDropdownAction(r));
        themeBtn = new MenuButton({340, 0, 90, f.size.y}, this, "Theme",
                                 state, new ToggleThemePickerAction(r));

        appendChild(view);
        appendChild(toggle);
        appendChild(shot);
        appendChild(plugins);
        appendChild(themeBtn);
    }

    const char *title() const override {
        return "Menu bar";
    }

    void layout() override {
        view->resize({100, texture->GetHeight()});
        toggle->resize({70, texture->GetHeight()});
        plugins->resize({80, texture->GetHeight()});
        shot->resize({90, texture->GetHeight()});
        themeBtn->resize({90, texture->GetHeight()});
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
