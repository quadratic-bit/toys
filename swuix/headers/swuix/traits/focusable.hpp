#pragma once
#include <swuix/widget.hpp>

class FocusableWidget : public virtual Widget {
public:
    FocusableWidget(Rect2f f, Widget *p, State *s) : Widget(f, p, s) {}

    virtual void focus() {};

    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *e) override;
};
