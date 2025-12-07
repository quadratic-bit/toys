#pragma once
#include <SDL3/SDL_keycode.h>
#include <string>
#include <vector>
#include <algorithm>

#include <swuix/widget.hpp>
#include <swuix/widgets/button.hpp>
#include <swuix/common.hpp>
#include <swuix/state.hpp>

struct MenuItemDesc {
    std::string label;
    Action     *action;
};

class PopupMenu;

class CloseAfterAction final : public Action {
    Action   *inner_;
    PopupMenu *menu_;
public:
    CloseAfterAction(Action *inner, PopupMenu *menu)
        : inner_(inner), menu_(menu) {}

    ~CloseAfterAction() override { delete inner_; }

    void apply(void *app_state, Widget *target) override;
};

class MenuItemButton final : public Button {
public:
    MenuItemButton(Rect2f f, Widget *p, const char *l, State *s, Action *a)
        : Button(f, p, l, s, a) {}

    void draw_idle() override {
        Rect2f f = frame();
        Rectangle *r = rectFill(state->window, f, {CLR_SURFACE_2});
        texture->Draw(*r);
    }

    void draw_hover() override {
        Rect2f f = frame();
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.08f);
        Rectangle *r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);
    }

    void draw_press() override {
        Rect2f f = frame();
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.12f);
        Rectangle *r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);
    }
};

class PopupMenu : public Widget {
    std::vector<MenuItemDesc> items_;
    float itemHeight_  = 22.0f;
    float padding_     = 4.0f;
    float minWidth_    = 140.0f;

public:
    PopupMenu(std::vector<MenuItemDesc> items,
              Rect2f f,
              Widget *p,
              State *s)
        : Widget(f, p, s), items_(std::move(items))
    {
        build();
    }

    const char *title() const override { return "PopupMenu"; }

    bool isClipped() const override { return false; }

    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *) override {
        if (!frame().Contains(ctx.mouse_rel)) {
            destroy();
            return CONSUME;
        }
        return PROPAGATE;
    }

    DispatchResult onKeyDown(DispatcherCtx, const KeyDownEvent *e) override {
        if (e && e->keycode == SDLK_ESCAPE) {
            destroy();
            return CONSUME;
        }
        return PROPAGATE;
    }

    void draw() override {
        Rect2f f = frame();
        texture->Clear({CLR_SURFACE_2});
        Rectangle *bg = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*bg);
    }

private:
    void build() {
        clearChildren();

        float x = padding_;
        float y = padding_;
        float w = std::max(minWidth_, frame().size.x - 2 * padding_);

        for (auto &it : items_) {
            Action *wrapped = new CloseAfterAction(it.action, this);
            Rect2f bf { x, y, w, itemHeight_ };

            auto *btn = new MenuItemButton(bf, this, it.label.c_str(), state, wrapped);
            appendChild(btn);

            y += itemHeight_;
        }

        texture->SetSize({ w + 2 * padding_, y + padding_ });
        requestLayout();
        requestRedraw();
    }

    friend class CloseAfterAction;
};
