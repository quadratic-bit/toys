#pragma once
#include <swuix/widgets/handle.hpp>
#include <swuix/widgets/container.hpp>
#include <swuix/widgets/button.hpp>

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

class Scrollbar;  // forward-declare

class ScrollbarSlider : public DraggableWidget {
    Scrollbar *scrollbar;

public:
    ScrollbarSlider(Rect2F f, Scrollbar *par, State *st);

    const char *title() const {
        return "Scrollbar slider";
    }

    DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);
    DispatchResult on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e);

    void render(Window *window, float off_x, float off_y);
};

class ScrollableWidget;  // forward-declare

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

    DispatchResult on_mouse_wheel(DispatcherCtx ctx, const MouseWheelEvent *e);
    DispatchResult on_mouse_move (DispatcherCtx ctx, const MouseMoveEvent *e);
};

class TallView : public MinimizableWidget, public ScrollableWidget, public ControlledContainer {
    TitleBar *titlebar;
    Scrollbar *scrollbar;

public:
    TallView(Rect2F content_frame_, Rect2F viewport_frame_, Widget *parent_, State *state_)
        : Widget(content_frame_, parent_, state_),
        MinimizableWidget(content_frame_, parent_, state_),
        ScrollableWidget(content_frame_, viewport_frame_, parent_, state_),
        ControlledContainer(content_frame_, parent_, state_) {
            titlebar = new TitleBar(state_);
            scrollbar = new Scrollbar(state_);

            // NOTE: don't change the order of the following two lines!
            // The line `titlebar->attach_to(...)` calls layout, which
            // depends on the scrollbar's host, which is not set until
            // scrollbar is attached
            scrollbar->attach_to(this);
            titlebar->attach_to(this);
        }

    DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
        if (!minimized) {
            return ControlledContainer::broadcast(ctx, e, reversed);
        }

        DispatcherCtx local_ctx = ctx.withOffset(frame);

        return titlebar->broadcast(local_ctx, e, reversed);
    }

    const char *title() const {
        return "Tall view";
    }

    DispatchResult on_layout(DispatcherCtx, const LayoutEvent *) {
        float progress_px = content_progress();
        titlebar->frame.y = progress_px - HANDLE_H;
        scrollbar->frame.y = progress_px;
        scrollbar->frame.h = viewport.h;
        scrollbar->slider->frame.h = scrollbar->scroll_height() * (viewport.h / frame.h);
        scrollbar->slider->frame.y = SCROLL_B_H + scrollbar->scroll_progress();
        scrollbar->refresh_layout();
        return PROPAGATE;
    }
};
