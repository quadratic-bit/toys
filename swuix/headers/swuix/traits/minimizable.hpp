#pragma once
#include <swuix/widget.hpp>

class MinimizableWidget : public virtual Widget {
public:
    bool minimized;
    MinimizableWidget(Rect2F dim_, Widget *parent_, State *state_)
        : Widget(dim_, parent_, state_), minimized(false) {}

    DispatchResult on_render(DispatcherCtx ctx, const RenderEvent *e) {
        if (minimized) return PROPAGATE;
        return Widget::on_render(ctx, e);
    }

    DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
        if (minimized) return PROPAGATE;
        return Widget::broadcast(ctx, e, reversed);
    }
};
