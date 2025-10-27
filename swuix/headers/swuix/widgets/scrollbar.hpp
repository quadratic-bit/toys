#pragma once
#include <swuix/traits/draggable.hpp>
#include <swuix/traits/scrollable.hpp>
#include <swuix/traits/controlled.hpp>

const float SCROLLBAR_W = 10.0f;
const float SCROLL_B_H  = 10.0f;

static inline const Rect2F scrollbar_box(Rect2F parent_box) {
    Rect2F box;
    box.x = parent_box.w - SCROLLBAR_W;
    box.y = 0.0f;
    box.w = SCROLLBAR_W;
    box.h = parent_box.h;
    return box;
}

static inline const Rect2F scrollbar_box_zero() {
    Rect2F box;
    box.x = -SCROLLBAR_W;
    box.y = 0.0f;
    box.w = SCROLLBAR_W;
    box.h = 0.0f;
    return box;
}

class Scrollbar;

class ScrollbarSlider : public DraggableWidget {
    Scrollbar *scrollbar;

public:
    ScrollbarSlider(Rect2F f, Scrollbar *par, State *st);

    const char *title() const {
        return "Scrollbar slider";
    }

    DispatchResult on_mouse_move(DispatcherCtx, const MouseMoveEvent *);
    DispatchResult on_mouse_down(DispatcherCtx, const MouseDownEvent *);

    void render(Window *window, float off_x, float off_y);
};

class Scrollbar : public Control, public WidgetContainer {
public:
    ScrollableWidget *host;
    ScrollbarSlider *slider;

    Scrollbar(State *state_);

    const char *title() const {
        return "Scrollbar";
    }

    void attach_to(ControlledWidget *host_);

    float scroll_height() {
        return frame.h - 2 * SCROLL_B_H;
    }

    float scroll_progress();

    void render(Window *window, float off_x, float off_y) {
        window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
        window->outline(frame, off_x, off_y, 2);
    }

    DispatchResult on_layout(DispatcherCtx, const LayoutEvent *) {
        if (children.size() < 3) return PROPAGATE;
        static float h = SCROLL_B_H;
        Widget *btn_up   = children[1];
        Widget *btn_down = children[2];
        btn_up->frame.y = 0;
        btn_down->frame.y = frame.h - h;
        return PROPAGATE;
    }
};
