#pragma once
#include <swuix/traits/minimizable.hpp>
#include <swuix/widgets/container.hpp>
#include <swuix/widgets/button.hpp>

class ScrollableWidget : public virtual Widget {
public:
    Rect2F viewport;

    ScrollableWidget(Rect2F content_frame_, Rect2F viewport_frame_, Widget *parent_, State *state_)
        : Widget(content_frame_, parent_, state_), viewport(viewport_frame_) {}

    Rect2F getViewport() const { return viewport; }

    float content_progress() const {
        return viewport.y - frame.y;
    }

    void set_frame(Rect2F new_frame) {
        float ydiff = viewport.y - frame.y;
        float xdiff = viewport.x - frame.x;
        frame = new_frame;
        viewport.x = frame.x + xdiff;
        viewport.y = frame.y + ydiff;
        refresh_layout();
    }

    void scroll_y(float dy) {
        frame.y = clamp(frame.y + dy, viewport.y + viewport.h - frame.h, viewport.y);
        refresh_layout();
    }

    DispatchResult on_mouse_wheel(DispatcherCtx, const MouseWheelEvent *);
    DispatchResult on_mouse_move (DispatcherCtx, const MouseMoveEvent  *);
};
