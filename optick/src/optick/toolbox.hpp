#pragma once
#include <swuix/widgets/handle.hpp>
#include <swuix/window/window.hpp>

#include "renderer.hpp"

class Move : public Action {
    Renderer *target;
    float dx;

public:
    Move(Renderer *w, float dx_) : target(w), dx(dx_) {}

    void apply(void *, Widget *) {
        target->move_camera(dx);
    }
};

class Rotate : public Action {
    Renderer *target;
    double dphi;

public:
    Rotate(Renderer *w, double dphi_cw) : target(w), dphi(dphi_cw) {}

    void apply(void *, Widget *) {
        target->rotate_camera_xz(dphi);
    }
};

class ControlPanel : public TitledContainer {
public:
	ControlPanel(Renderer *renderer, Rect2F rect, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), TitledContainer(rect, parent_, state_) {
        Button *move_left  = new Button(frect(20, 45, 25, 25), NULL, "<", state, new Move(renderer, -0.5));
        Button *move_right = new Button(frect(50, 45, 25, 25), NULL, ">", state, new Move(renderer,  0.5));

        Button *rotate_cw  = new Button(frect(60, 10, 30, 25), NULL, "CW",  state, new Rotate(renderer, -M_PI / 20.0));
        Button *rotate_ccw = new Button(frect( 5, 10, 30, 25), NULL, "CCW", state, new Rotate(renderer,  M_PI / 20.0));

		Widget *btns[] = { move_left, move_right, rotate_cw, rotate_ccw };
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
