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

class TogglePropertiesPanelAction final : public Action {
    Desktop *root_;
public:
    TogglePropertiesPanelAction(Desktop *r) : root_(r) {}
    void apply(void *, Widget *) override;
};

class ToggleControlsPanelAction final : public Action {
    Desktop *root_;
public:
    ToggleControlsPanelAction(Desktop *r) : root_(r) {}
    void apply(void *, Widget *) override;
};

class ToggleFileDropdownAction final : public Action {
    Desktop *root_;
public:
    ToggleFileDropdownAction(Desktop *r) : root_(r) {}
    void apply(void *, Widget *target) override;
};

class ToggleViewDropdownAction final : public Action {
    Desktop *root_;
public:
    ToggleViewDropdownAction(Desktop *r) : root_(r) {}
    void apply(void *, Widget *target) override;
};

class MenuButton final : public Button {
    void _draw_separator() {
        Rect2f f = frame();
        dr4::Line *l = thickLine(state->window, {f.size.x - 1, 0}, {f.size.x - 1, f.size.y - 2}, {CLR_BORDER_SUBTLE}, 1);
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
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_3), 0.06);
        r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);

        _draw_separator();
    }

    void draw_press() override {
        Rect2f f = frame();
        Rectangle *r;
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_3), 0.10);
        r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);

        _draw_separator();
    }

    void draw_idle() override {
        Rect2f f = frame();
        Rectangle *r;
        r = rectFill(state->window, f, {CLR_SURFACE_3});
        texture->Draw(*r);

        _draw_separator();
    }
};

class MenuBar final : public Widget {
    Button *file;
    Button *view;
    Button *plugins;

public:
    MenuBar(Rect2f f, Widget *p, State *s, Desktop *r) : Widget(f, p, s) {
        file    = new MenuButton({0,   0, 70, f.size.y}, this, "File",    state, new ToggleFileDropdownAction(r));
        view    = new MenuButton({70,  0, 70, f.size.y}, this, "View",    state, new ToggleViewDropdownAction(r));
        plugins = new MenuButton({140, 0, 90, f.size.y}, this, "Plugins", state, new TogglePluginsDropdownAction(r));

        appendChild(file);
        appendChild(view);
        appendChild(plugins);
    }

    const char *title() const override {
        return "Menu bar";
    }

    void layout() override {
        file->resize({70, texture->GetHeight()});
        view->resize({70, texture->GetHeight()});
        plugins->resize({90, texture->GetHeight()});
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
