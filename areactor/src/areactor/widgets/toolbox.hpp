#pragma once
#include <swuix/widgets/handle.hpp>
#include <swuix/window/window.hpp>

#include "state.hpp"

static void cb_add(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_speed(+40);
}

static void cb_sub(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_speed(-40);
}

static void cb_add_particles(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->reactor->add_particles(((ReactorState*)st)->now, 5);
}

static void cb_delete_particles(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->reactor->remove_particles(5);
}

static void cb_hotter(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_temp(Side::LEFT, +0.50);
}

static void cb_colder(void *st, Widget *d) {
	(void)d;
	((ReactorState*)st)->add_to_wall_temp(Side::LEFT, -0.50);
}


class ToolboxWidget : public HandledContainer {
public:
	ToolboxWidget(FRect rect, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), HandledContainer(rect, parent_, state_) {
		Button *btn_left    = new Button(frect(20, 10, 50, 25), NULL, "<-",   state, cb_sub);
		Button *btn_right   = new Button(frect(80, 10, 50, 25), NULL, "->",   state, cb_add);

		Button *btn_cold    = new Button(frect(20, 40, 50, 25), NULL, "Cold", state, cb_colder);
		Button *btn_hot     = new Button(frect(80, 40, 50, 25), NULL, "Hot",  state, cb_hotter);

		Button *btn_add     = new Button(frect(20, 70, 50, 25), NULL, "+",    state, cb_add_particles);
		Button *btn_remove  = new Button(frect(80, 70, 50, 25), NULL, "-",    state, cb_delete_particles);

		Widget *btns[] = { btn_left, btn_right, btn_cold, btn_hot, btn_add, btn_remove };
		this->append_children(Widget::make_children(btns));
	}

	const char *title() const {
		return "Toolbox";
	}

	void render_body(Window *window, int off_x, int off_y) {
		// body
		window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
		window->outline(frame, off_x, off_y, 2);

		// children
		HandledContainer::render_body(window, off_x, off_y);
	}
};

