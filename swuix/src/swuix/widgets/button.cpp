#include <swuix/widgets/button.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

DispatchResult Button::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	bool c = contains_point(ctx);
	bool not_covered_or_doesnt_contain = state->mouse.target || !c;
	if (hovered && not_covered_or_doesnt_contain) {
		hovered = false;
	}
	if (c && !state->mouse.target) {
		hovered = true;
		state->mouse.target = this;
	}
        return PROPAGATE;
}

DispatchResult Button::on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e) {
	(void)e;
	(void)ctx;
	if (state->mouse.target != this) return PROPAGATE;
	pressed = true;
	state->mouse.capture = this;
	if (action) action->apply(state, this);
        return CONSUME;
}

DispatchResult Button::on_mouse_up(DispatcherCtx ctx, const MouseUpEvent *e) {
	(void)e;
	(void)ctx;
	if (pressed) pressed = false;
	return PROPAGATE;
}
