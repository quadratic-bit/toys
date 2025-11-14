#pragma once
#include <swuix/widget.hpp>

class DraggableWidget : public virtual Widget {
protected:
    dr4::Vec2f start_drag;
    dr4::Vec2f start_drag_pos;
    bool is_dragging;

public:
    DraggableWidget(Rect2f f, Widget *p, State *s)
        : Widget(f, p, s), start_drag(), start_drag_pos(), is_dragging(false) {}

    DispatchResult onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) override;
    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *e) override;
    DispatchResult onMouseUp  (DispatcherCtx ctx, const MouseUpEvent   *e) override;
};
