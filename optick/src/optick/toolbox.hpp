#pragma once
#include <swuix/widgets/handle.hpp>
#include <swuix/window/window.hpp>

#include "renderer.hpp"

class Scroll : public Action {
    Renderer *target;
    float dx;

public:
    Scroll(Renderer *w, float dx_) : target(w), dx(dx_) {}

    void apply(void *, Widget *) {
        target->move_camera_left(dx);
    }
};

class ControlPanel : public TitledContainer {
public:
	ControlPanel(Renderer *renderer, Rect2F rect, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), TitledContainer(rect, parent_, state_) {
        Button *move_left = new Button(frect(20, 20, 25, 25), NULL, "<", state, new Scroll(renderer, -0.5));
        Button *move_right = new Button(frect(50, 20, 25, 25), NULL, ">", state, new Scroll(renderer, 0.5));

		Widget *btns[] = { move_left, move_right };
		this->append_children(Widget::makeChildren(btns));
	}

	const char *title() const {
		return "Controller";
	}

	void render(Window *window, float off_x, float off_y) {
		window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
		window->outline(frame, off_x, off_y, 2);
	}
};
