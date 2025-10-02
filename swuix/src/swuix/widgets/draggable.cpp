#include <swuix/widgets/draggable.hpp>
#include <swuix/state.hpp>

DispatchResult DraggableWidget::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (state->mouse.state == MouseState::Dragging && is_dragging) {
		frame.x = ctx.mouse_rel.x - start_drag_x;
		frame.y = ctx.mouse_rel.y - start_drag_y;
	}
        return CONSUME;
}

DispatchResult DraggableWidget::on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e) {
	(void)e;
	if (state->mouse.target == this) {
		is_dragging = true;
		start_drag_x = ctx.mouse_rel.x;
		start_drag_y = ctx.mouse_rel.y;
		return CONSUME;
	}
	return PROPAGATE;
}

DispatchResult DraggableWidget::on_mouse_up(DispatcherCtx ctx, const MouseUpEvent *e) {
	(void)e;
	(void)ctx;
	if (is_dragging) is_dragging = false;
	return PROPAGATE;
}
