#pragma once
#include <swuix/widget.hpp>

class DraggableWidget : public virtual Widget {
protected:
	float start_drag_x;  // x position of `parent`
	float start_drag_y;  // y position of `parent`
	bool is_dragging;
public:
	DraggableWidget(FRect f, Widget *par, State *st)
		: Widget(f, par, st), start_drag_x(0), start_drag_y(0), is_dragging(false) {}

	DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);
	DispatchResult on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e);
	DispatchResult on_mouse_up  (DispatcherCtx ctx, const MouseUpEvent   *e);
};
