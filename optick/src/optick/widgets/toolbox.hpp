#pragma once
#include "dr4/math/color.hpp"
#include "renderer.hpp"

class Strafe : public Action {
    Renderer *target;
    float dright;

public:
    Strafe(Renderer *w, float dright_) : target(w), dright(dright_) {}

    void apply(void *, Widget *) {
        target->getCamera()->strafe(dright);
    }
};

class Move : public Action {
    Renderer *target;
    float dfwd;

public:
    Move(Renderer *w, float dfwd_) : target(w), dfwd(dfwd_) {}

    void apply(void *, Widget *) {
        target->getCamera()->move(dfwd);
    }
};

class Elevate : public Action {
    Renderer *target;
    float dup;

public:
    Elevate(Renderer *w, float dup_) : target(w), dup(dup_) {}

    void apply(void *, Widget *) {
        target->getCamera()->elevate(dup);
    }
};

class Yaw : public Action {
    Renderer *target;
    double dphi;

public:
    Yaw(Renderer *w, double dphi_cw) : target(w), dphi(dphi_cw) {}

    void apply(void *, Widget *) {
        target->getCamera()->yaw(dphi);
    }
};

class Pitch : public Action {
    Renderer *target;
    double dphi;

public:
    Pitch(Renderer *w, double dphi_up) : target(w), dphi(dphi_up) {}

    void apply(void *, Widget *) {
        target->getCamera()->pitch(dphi);
    }
};

/* CCW      CW
 *      ^      Up
 *    <   >
 *      v     Down
 *  -      +
 */
class ControlPanel final : public TitledWidget {
public:
	ControlPanel(Renderer *renderer, Rect2f f, Widget *p, State *s)
			: Widget(f, p, s), TitledWidget(f, p, s) {
        Button *move_right = new Button({80, 55, 25, 25}, NULL, ">", state, new Strafe(renderer,  0.5));
        Button *move_left  = new Button({20, 55, 25, 25}, NULL, "<", state, new Strafe(renderer, -0.5));

        Button *move_fwd  = new Button({50, 25, 25, 25}, NULL, "^", state, new Move(renderer,  0.5));
        Button *move_back = new Button({50, 90, 25, 25}, NULL, "v", state, new Move(renderer, -0.5));

        Button *move_up   = new Button({90, 105, 25, 25}, NULL, "+", state, new Elevate(renderer,  0.5));
        Button *move_down = new Button({10, 105, 25, 25}, NULL, "-", state, new Elevate(renderer, -0.5));

        Button *yaw_cw  = new Button({90, 10, 30, 25}, NULL, "CW",  state, new Yaw(renderer, -10));
        Button *yaw_ccw = new Button({ 5, 10, 30, 25}, NULL, "CCW", state, new Yaw(renderer,  10));

        Button *pitch_up   = new Button({130, 25, 30, 25}, NULL, "Up",   state, new Pitch(renderer,  10));
        Button *pitch_down = new Button({125, 90, 40, 25}, NULL, "Down", state, new Pitch(renderer, -10));

		Widget *btns[] = {
            move_left, move_right,
            move_fwd,  move_back,
            move_up,   move_down,
            yaw_cw,    yaw_ccw,
            pitch_up,  pitch_down };
        for (Widget *btn : btns) {
            this->appendChild(btn);
        }
	}

	const char *title() const override {
		return "Control";
	}

	void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_1}, 2, {CLR_BORDER});
        texture->Draw(*r);
	}
};
