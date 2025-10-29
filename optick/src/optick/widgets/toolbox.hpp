#pragma once
#include "renderer.hpp"

class Strafe : public Action {
    Renderer *target;
    float dright;

public:
    Strafe(Renderer *w, float dright_) : target(w), dright(dright_) {}

    void apply(void *, Widget *) {
        target->strafeRight(dright);
    }
};

class Move : public Action {
    Renderer *target;
    float dfwd;

public:
    Move(Renderer *w, float dfwd_) : target(w), dfwd(dfwd_) {}

    void apply(void *, Widget *) {
        target->moveForward(dfwd);
    }
};

class Elevate : public Action {
    Renderer *target;
    float dup;

public:
    Elevate(Renderer *w, float dup_) : target(w), dup(dup_) {}

    void apply(void *, Widget *) {
        target->moveUp(dup);
    }
};

class Yaw : public Action {
    Renderer *target;
    double dphi;

public:
    Yaw(Renderer *w, double dphi_cw) : target(w), dphi(dphi_cw) {}

    void apply(void *, Widget *) {
        target->yaw(dphi);
    }
};

class Pitch : public Action {
    Renderer *target;
    double dphi;

public:
    Pitch(Renderer *w, double dphi_up) : target(w), dphi(dphi_up) {}

    void apply(void *, Widget *) {
        target->pitch(dphi);
    }
};

/* CCW      CW
 *      ^      Up
 *    <   >
 *      v     Down
 *  -      +
 */
class ControlPanel : public TitledContainer {
public:
	ControlPanel(Renderer *renderer, Rect2F rect, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), TitledContainer(rect, parent_, state_) {
        Button *move_right = new Button(frect(80, 55, 25, 25), NULL, ">", state, new Strafe(renderer,  0.5));
        Button *move_left  = new Button(frect(20, 55, 25, 25), NULL, "<", state, new Strafe(renderer, -0.5));

        Button *move_fwd  = new Button(frect(50, 25, 25, 25), NULL, "^", state, new Move(renderer,  0.5));
        Button *move_back = new Button(frect(50, 90, 25, 25), NULL, "v", state, new Move(renderer, -0.5));

        Button *move_up   = new Button(frect(90, 105, 25, 25), NULL, "+", state, new Elevate(renderer,  0.5));
        Button *move_down = new Button(frect(10, 105, 25, 25), NULL, "-", state, new Elevate(renderer, -0.5));

        Button *yaw_cw  = new Button(frect(90, 10, 30, 25), NULL, "CW",  state, new Yaw(renderer, -10));
        Button *yaw_ccw = new Button(frect( 5, 10, 30, 25), NULL, "CCW", state, new Yaw(renderer,  10));

        Button *pitch_up   = new Button(frect(130, 25, 30, 25), NULL, "Up",   state, new Pitch(renderer,  10));
        Button *pitch_down = new Button(frect(125, 90, 40, 25), NULL, "Down", state, new Pitch(renderer, -10));

		Widget *btns[] = {
            move_left, move_right,
            move_fwd,  move_back,
            move_up,   move_down,
            yaw_cw,    yaw_ccw,
            pitch_up,  pitch_down };
		this->append_children(Widget::makeChildren(btns));
	}

	const char *title() const {
		return "Control";
	}

	void render(Window *window, float off_x, float off_y) {
		window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
		window->outline(frame, off_x, off_y, 2);
	}
};
