#include "swuix/common.hpp"
#include <swuix/widgets/button.hpp>
#include <swuix/state.hpp>

DispatchResult Button::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *) {
    bool c = containsMouse(ctx);
    bool not_covered_or_doesnt_contain = state->mouse.target || !c;
    if (hovered && not_covered_or_doesnt_contain) {
        hovered = false;
        requestRedraw();
    }
    if (c && !state->mouse.target) {
        hovered = true;
        state->mouse.target = this;
        requestRedraw();
    }
    return PROPAGATE;
}

DispatchResult Button::onMouseDown(DispatcherCtx, const MouseDownEvent *) {
    if (state->mouse.target != this) return PROPAGATE;
    pressed = true;
    requestRedraw();
    state->mouse.capture = this;
    if (action) action->apply(state, this);

    return CONSUME;
}

DispatchResult Button::onMouseUp(DispatcherCtx, const MouseUpEvent *) {
    if (pressed) {
        pressed = false;
        requestRedraw();
    }
    if (state->mouse.capture == this) {
        state->mouse.capture = nullptr;
        return CONSUME;
    };
    return PROPAGATE;
}

void Button::draw() {
    Rect2f f = frame();

    Rectangle *r;
    if (pressed) {
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.10);
        r = rectBorder(state->window, f, {d.r, d.g, d.b}, 1, {CLR_BORDER});
    } else if (hovered) {
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.06);
        r = rectBorder(state->window, f, {d.r, d.g, d.b}, 1, {CLR_BORDER});
    } else {
        r = rectBorder(state->window, f, {CLR_SURFACE_2}, 1, {CLR_BORDER});
    }
    texture->Draw(*r);

    Text *t = textAligned(state->window, label, f.size / 2.0f, Color(CLR_TEXT_STRONG), state->appfont, 16, HAlign::CENTER);
    texture->Draw(*t);
}
