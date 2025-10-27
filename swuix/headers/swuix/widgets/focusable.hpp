#pragma once
#include <swuix/widget.hpp>

class FocusableWidget : public virtual Widget {
public:
    FocusableWidget(Rect2F content_frame_, Widget *parent_, State *state_)
        : Widget(content_frame_, parent_, state_) {}

    virtual void focus() {};

    DispatchResult on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e);
};
