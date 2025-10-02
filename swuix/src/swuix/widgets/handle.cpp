#include <swuix/widgets/handle.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

DispatchResult Handle::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (state->mouse.state == MouseState::Dragging && is_dragging) {
		parent->frame.x += ctx.mouse_rel.x - start_drag_x;
		parent->frame.y += ctx.mouse_rel.y - start_drag_y;
	}
	return Widget::on_mouse_move(ctx, e);
}
