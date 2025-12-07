#pragma once
#include <swuix/widget.hpp>
#include <swuix/common.hpp>
#include <swuix/state.hpp>

#include "canvas_theme_picker.hpp"

class ColorPickerPopup final : public Widget {
    CanvasThemePicker *picker_{nullptr};

public:
    ColorPickerPopup(Rect2f f, Widget *p, State *s, Canvas *canvas)
        : Widget(f, p, s)
    {
        picker_ = new CanvasThemePicker(canvas, {0, 0, f.size.x, f.size.y}, this, state);
        appendChild(picker_);
        requestLayout();
    }

    const char *title() const override { return "ColorPickerPopup"; }
    bool isClipped() const override { return false; }

    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *e) override {
        if (!frame().Contains(ctx.mouse_rel)) {
            destroy();
            return CONSUME;
        }
        return Widget::onMouseDown(ctx, e);
    }

    DispatchResult onKeyDown(DispatcherCtx, const KeyDownEvent *e) override {
        if (e && e->keycode == dr4::KEYCODE_ESCAPE) {
            destroy();
            return CONSUME;
        }
        return PROPAGATE;
    }

    void layout() override {
        if (!picker_) return;
        Rect2f f = frame();
        picker_->position = {0, 0};
        picker_->texture->SetSize(f.size.x, f.size.y);
    }

    void draw() override {
        Rect2f f = frame();
        texture->Clear({CLR_SURFACE_2});
        Rectangle *bg = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*bg);
    }
};
