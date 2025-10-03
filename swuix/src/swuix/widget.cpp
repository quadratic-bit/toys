#include <swuix/widget.hpp>
#include <swuix/state.hpp>

DispatchResult Widget::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (!state->mouse.target && contains_point(ctx)) {
		state->mouse.target = this;
	}
	return PROPAGATE;
}
