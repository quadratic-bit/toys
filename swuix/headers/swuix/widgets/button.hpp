#pragma once
#include <swuix/widget.hpp>

static const int BTN_THICK = 1;

class BtnCallbackAction : public Action {
	void (*click_cb)(void*, Widget*);
public:
	BtnCallbackAction(void (*click_cb_)(void*, Widget*)) : click_cb(click_cb_) {}

	void apply(void *state, Widget *target) {
		if (click_cb) click_cb(state, target);
	}
};

class Button : public Widget {
	bool hovered;
	bool pressed;
	Action *action;
	const char* label;
public:
	Button(FRect f, Widget *par, const char *label_, State *st, Action *action_)
		: Widget(f, par, st), hovered(false), action(action_), label(label_) {}

	Button(FRect f, Widget *par, const char *label_, State *st, void (*on_click_)(void*, Widget*))
			: Widget(f, par, st), hovered(false), label(label_) {
		action = new BtnCallbackAction(on_click_);
	}
	~Button() {
		delete action;
	}

	const char *title() const {
		return label;
	}

	DispatchResult on_mouse_move(DispatcherCtx, const MouseMoveEvent *);
	DispatchResult on_mouse_down(DispatcherCtx, const MouseDownEvent *);
	DispatchResult on_mouse_up  (DispatcherCtx, const MouseUpEvent   *);

	void render(Window *window, float off_x, float off_y) {
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
