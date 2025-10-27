#pragma once
#include <swuix/traits/controlled.hpp>
#include <swuix/traits/minimizable.hpp>
#include <swuix/traits/draggable.hpp>
#include <swuix/widgets/button.hpp>

const float HANDLE_H = 20.0f;

static inline const Rect2F handle_box(Rect2F parent_box) {
    Rect2F box;
    box.x = 0.0f;
    box.y = -HANDLE_H;
    box.w = parent_box.w;
    box.h = HANDLE_H;
    return box;
}

static inline const Rect2F handle_box_zero() {
    Rect2F box;
    box.x = 0.0f;
    box.y = -HANDLE_H;
    box.w = 0.0f;
    box.h = HANDLE_H;
    return box;
}

class TitleBar : public Control, public DraggableWidget, public WidgetContainer {
    Button *btn_minimize;
    MinimizableWidget *host;

public:
    TitleBar(State *state_);

    DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);

    DispatchResult on_layout(DispatcherCtx, const LayoutEvent *);

    void attach_to(ControlledWidget *host_);

    const char *title() const {
        return "Title bar";
    }

    void render(Window *window, float off_x, float off_y);
};
