#pragma once
#include "swuix/common.hpp"
#include "swuix/traits/draggable.hpp"
#include "swuix/widget.hpp"
#include "swuix/state.hpp"

class WRect final : public DraggableWidget {
public:
    WRect(Rect2f frame, Widget *p, State *s) : Widget(frame, p, s), DraggableWidget(frame, p, s) {}

    const char *title() const override {
        return "{Canvas}Rect";
    }

    void draw() override {
        dr4::Rectangle *r = outline(state->window, frame(), 3, {255, 0, 0});
        texture->Draw(*r);
    }
};

class Canvas final : public Widget {
public:
    Canvas(Rect2f frame, Widget *p, State *s) : Widget(frame, p, s) {}

    const char *title() const override {
        return "Canvas";
    }

    void draw() override {
        texture->Clear({0, 0, 0, 0});
    }

    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *) override {
        if (state->mouse.target != this) return PROPAGATE;

        WRect *r = new WRect({ctx.mouse_rel - dr4::Vec2f{25, 25}, {50, 50}}, NULL, state);
        appendChild(r);
        requestRedraw();

        return CONSUME;
    }
};
