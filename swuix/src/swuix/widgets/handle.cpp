#include <swuix/widgets/handle.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

DispatchResult Handle::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (state->mouse.state == MouseState::Dragging && is_dragging) {
		parent->frame.x += ctx.local.x - start_drag_x;
		parent->frame.y += ctx.local.y - start_drag_y;
	}
	return Widget::on_mouse_move(ctx, e);
}

void Handle::render(Window *window, int off_x, int off_y) {
	window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
	window->outline(frame, off_x, off_y, 2);
	WidgetContainer::render(window, off_x, off_y);
}

void Handle::adjust_width(float new_width) {
	frame.w = new_width;
	child_at(0)->frame.x = frame.w - 20;
}

void HandledWidget::render(Window *window, int off_x, int off_y) {
	if (!minimized) render_body(window, off_x, off_y);
	handle->render(window, frame.x + off_x, frame.y + off_y);
}

void HandledContainer::render(Window *window, int off_x, int off_y) {
	HandledWidget::render(window, off_x, off_y);
}

void HandledContainer::render_body(Window *window, int off_x, int off_y) {
	// body
	window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
	window->outline(frame, off_x, off_y, 2);

	// children
	WidgetContainer::render(window, off_x, off_y);
}
