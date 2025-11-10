#pragma once
#include <swuix/widget.hpp>

class MinimizableWidget : public virtual Widget {
public:
    bool minimized;

    MinimizableWidget(Rect2f f, Widget *p, State *s)
        : Widget(f, p, s), minimized(false) {}

    virtual void blit(Texture *target, Vec2f acc) override {
        if (!minimized) Widget::blit(target, acc);
    }

    virtual DispatchResult broadcast(DispatcherCtx ctx, Event *e) override {
        if (minimized) return PROPAGATE;
        return Widget::broadcast(ctx, e);
    }
};
