#pragma once
#include <cstdio>
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

    virtual void draw() override {
        Rect2f f = frame();
        // TODO: color without alpha
        texture->Clear(Color(CLR_BORDER, 255));

        Rectangle r;
        if (pressed) {
            const RGBu8 darkened = OKLabDarken(RGB(CLR_SURFACE_2), 0.10);
            r = {
                Rect2f(BTN_THICK, BTN_THICK, f.size.x - BTN_THICK * 2, f.size.y - BTN_THICK * 2),
                Color(darkened.r, darkened.g, darkened.b, 225)
            };
        } else if (hovered) {
            const RGBu8 darkened = OKLabDarken(RGB(CLR_SURFACE_2), 0.06);
            r = {
                Rect2f(BTN_THICK, BTN_THICK, f.size.x - BTN_THICK * 2, f.size.y - BTN_THICK * 2),
                Color(darkened.r, darkened.g, darkened.b, 225)
            };
        } else {
            r = {
                Rect2f(BTN_THICK, BTN_THICK, f.size.x - BTN_THICK * 2, f.size.y - BTN_THICK * 2),
                Color(CLR_SURFACE_2, 225)
            };
        }
        texture->Draw(r);

        Text t;
        t.text = label;
        t.color = Color(CLR_TEXT_STRONG, 255);
        // TODO: align t.pos = {f.size.x / 2, f.size.y / 2};
        t.pos = {5, 5};
        t.valign = Text::VAlign::MIDDLE;
        t.font = nullptr;
        texture->Draw(t);
    }

    DispatchResult onMouseMove(DispatcherCtx, const MouseMoveEvent *) override;
    DispatchResult onMouseDown(DispatcherCtx, const MouseDownEvent *) override;
    DispatchResult onMouseUp  (DispatcherCtx, const MouseUpEvent   *) override;
};
