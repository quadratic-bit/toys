#pragma once
#include <swuix/widget.hpp>
#include <swuix/window/common.hpp>

struct Action {
    virtual ~Action() {}
    virtual void apply(void*, Widget*) = 0;
};

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

    const char *label;

protected:
    virtual void draw_hover();
    virtual void draw_press();
    virtual void draw_idle();
    virtual void draw_text();

    bool align_left = false;

public:
    Button(Rect2f f, Widget *p, const char *l, State *s, Action *a)
        : Widget(f, p, s), hovered(false), pressed(false), action(a), label(l) {}

    Button(Rect2f f, Widget *p, const char *l, State *s, void (*on_click)(void*, Widget*))
        : Widget(f, p, s), hovered(false), pressed(false), action(new BtnCallbackAction(on_click)), label(l) {}

    Button(Rect2f f, Widget *p, const char *l, State *s)
        : Widget(f, p, s), hovered(false), pressed(false), action(new BtnCallbackAction(nullptr)), label(l) {}

    ~Button() {
        delete action;
    }

    void setAction(Action *a) {
        delete action;
        action = a;
    }

    const char *title() const override {
        return label;
    }

    void draw() override final;

    DispatchResult onMouseMove(DispatcherCtx, const MouseMoveEvent *) override;
    DispatchResult onMouseDown(DispatcherCtx, const MouseDownEvent *) override;
    DispatchResult onMouseUp  (DispatcherCtx, const MouseUpEvent   *) override;
};
