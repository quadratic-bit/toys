#include "button.hpp"
#include "../window.hpp"
#include "../state.hpp"

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
	SDL_FRect outer = frect(frame.x + off_x - BTN_THICK, frame.y + off_y - BTN_THICK,
				frame.w + BTN_THICK * 2, frame.h + BTN_THICK * 2);
	SDL_FRect inner = frect(frame.x + off_x + BTN_THICK, frame.y + off_y + BTN_THICK,
				frame.w - BTN_THICK * 2, frame.h - BTN_THICK * 2);

	SDL_SetRenderDrawColor(window->renderer, CLR_BLACK, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(window->renderer, &outer);

	if (hovered) SDL_SetRenderDrawColor(window->renderer, CLR_LIGHT_GRAY, SDL_ALPHA_OPAQUE);
	else SDL_SetRenderDrawColor(window->renderer, CLR_PLATINUM, SDL_ALPHA_OPAQUE);
	SDL_RenderFillRect(window->renderer, &inner);

	window->text_aligned(label, frame.x + off_x + frame.w / 2, frame.y + off_y + frame.h / 2, TA_CENTER);
}
