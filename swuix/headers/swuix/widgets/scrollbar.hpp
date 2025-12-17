#pragma once
#include <cassert>

#include <swuix/window/common.hpp>
#include <swuix/traits/draggable.hpp>
#include <swuix/traits/scrollable.hpp>

const float SCROLLBAR_W  = 10.0f;
const float SCROLL_BUT_H = 10.0f;

class VScrollbar;

class VScrollbarSlider final : public DraggableWidget {
    VScrollbar *scrollbar;

public:
    VScrollbarSlider(Rect2f, VScrollbar *, State *);

    const char *title() const override {
        return "Scrollbar slider";
    }

    DispatchResult onMouseMove(DispatcherCtx, const MouseMoveEvent *) override;
    DispatchResult onMouseDown(DispatcherCtx, const MouseDownEvent *) override;

    void draw() override;
};

class VScrollbar final : public Widget {
public:
    ScrollableWidget *host;
    VScrollbarSlider *slider;

    VScrollbar(State*);

    const char *title() const override {
        return "Scrollbar";
    }

    void attachTo(ScrollableWidget *);

    float scrollHeight() const {
        return texture->GetHeight() - 2 * SCROLL_BUT_H;
    }

    float scrollProgress() const;

    void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);
    }

    void layout() override {
        assert(children.size() >= 3);
        static float h = SCROLL_BUT_H;
        Widget *btn_up   = children[1];
        Widget *btn_down = children[2];
        btn_up->position.y = 0;
        btn_down->position.y = texture->GetHeight() - h;
    }
};
