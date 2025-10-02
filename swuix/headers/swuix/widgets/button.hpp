#pragma once
#include <swuix/widget.hpp>

static const int BTN_THICK = 1;

class Button : public Widget {
	bool hovered;
	bool pressed;
	void (*click_cb)(void*, Widget*);
	const char* label;
public:
	Button(FRect f, Widget *par, const char *label_, State *st, void (*on_click_)(void*, Widget*))
		: Widget(f, par, st), hovered(false), click_cb(on_click_), label(label_) {}

	bool is_leaf() const { return true; }

	const char *title() const {
		return label;
	}

	DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);
	DispatchResult on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e);
	DispatchResult on_mouse_up  (DispatcherCtx ctx, const MouseUpEvent   *e);

	void render(Window *window, int off_x, int off_y) {
		FRect outer = frect(frame.x + off_x - BTN_THICK, frame.y + off_y - BTN_THICK,
				frame.w + BTN_THICK * 2, frame.h + BTN_THICK * 2);
		FRect inner = frect(frame.x + off_x + BTN_THICK, frame.y + off_y + BTN_THICK,
				frame.w - BTN_THICK * 2, frame.h - BTN_THICK * 2);

		window->draw_filled_rect_rgb(outer, CLR_BLACK);

		if (hovered) window->draw_filled_rect_rgb(inner, CLR_LIGHT_GRAY);
		else window->draw_filled_rect_rgb(inner, CLR_PLATINUM);

		window->text_aligned(label, frame.x + off_x + frame.w / 2, frame.y + off_y + frame.h / 2, TA_CENTER);
	}
};
