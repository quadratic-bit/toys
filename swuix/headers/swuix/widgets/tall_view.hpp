#pragma once
#include <swuix/traits/scrollable.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/widgets/titlebar.hpp>

class TallView : public MinimizableWidget, public ScrollableWidget, public ControlledContainer {
    TitleBar *titlebar;
    Scrollbar *scrollbar;

public:
    TallView(Rect2F content_frame_, Rect2F viewport_frame_, Widget *parent_, State *state_)
            : Widget(content_frame_, parent_, state_),
            MinimizableWidget(content_frame_, parent_, state_),
            ScrollableWidget(content_frame_, viewport_frame_, parent_, state_),
            ControlledContainer(content_frame_, parent_, state_) {
        titlebar  = new TitleBar(state_);
        scrollbar = new Scrollbar(state_);

        // NOTE: don't change the order of the following two lines!
        // The line `titlebar->attach_to(...)` calls layout, which
        // depends on the scrollbar's host, which is not set until
        // scrollbar is attached
        scrollbar->attach_to(this);
        titlebar ->attach_to(this);
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
