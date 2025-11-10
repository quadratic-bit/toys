#pragma once
#include <swuix/widget.hpp>

class DraggableWidget : public virtual Widget {
protected:
    float start_drag_x;  // x position of `parent`
    float start_drag_y;  // y position of `parent`
    bool is_dragging;
public:
    DraggableWidget(Rect2f f, Widget *p, State *s)
        : Widget(f, p, s), start_drag_x(0), start_drag_y(0), is_dragging(false) {}

    DispatchResult onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) override;
    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *e) override;
    DispatchResult onMouseUp  (DispatcherCtx ctx, const MouseUpEvent   *e) override;
};
