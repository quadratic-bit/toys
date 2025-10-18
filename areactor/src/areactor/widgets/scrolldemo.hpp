#pragma once
#include <cstdio>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/window/window.hpp>

#include "state.hpp"

static void cb_nothing(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_temp(Side::LEFT, -0.50);
}

class ScrollDemo : public TallView {
public:
	ScrollDemo(Rect2F rect, Rect2F clip, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), TallView(rect, clip, parent_, state_) {
		Button *btn_nothing  = new Button(frect(20, 10, 50, 25), NULL, "btn1", state, cb_nothing);
		Button *btn_nothing2 = new Button(frect(20, 110, 50, 25), NULL, "btn2", state, cb_nothing);

		Widget *btns[] = { btn_nothing, btn_nothing2 };
		this->append_children(Widget::makeChildren(btns));
	}

	const char *title() const {
		return "Scroll";
	}

	void render(Window *window, float off_x, float off_y) {
		// body
		window->clear_rect(viewport, off_x, off_y, CLR_TIMBERWOLF);

		// children
		window->clip(viewport);
		window->text("text..", frame.x + off_x + 20, frame.y + off_y + 60);
		//TallView::render(window, off_x, off_y);
		window->unclip();

		window->outline(viewport, off_x, off_y, 2);
	}
};
