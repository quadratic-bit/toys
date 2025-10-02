#include <swuix/widgets/button.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

DispatchResult Button::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	bool c = contains_point(ctx.local);
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
	click_cb(state, this);
        return CONSUME;
}

DispatchResult Button::on_mouse_up(DispatcherCtx ctx, const MouseUpEvent *e) {
	(void)e;
	(void)ctx;
	if (pressed) pressed = false;
	return PROPAGATE;
}

void Button::render(Window *window, int off_x, int off_y) {
	FRect outer = frect(frame.x + off_x - BTN_THICK, frame.y + off_y - BTN_THICK,
			frame.w + BTN_THICK * 2, frame.h + BTN_THICK * 2);
	FRect inner = frect(frame.x + off_x + BTN_THICK, frame.y + off_y + BTN_THICK,
				frame.w - BTN_THICK * 2, frame.h - BTN_THICK * 2);

	window->draw_filled_rect_rgb(outer, CLR_BLACK);

	if (hovered) window->draw_filled_rect_rgb(inner, CLR_LIGHT_GRAY);
	else window->draw_filled_rect_rgb(inner, CLR_PLATINUM);

	window->text_aligned(label, frame.x + off_x + frame.w / 2, frame.y + off_y + frame.h / 2, TA_CENTER);
}
