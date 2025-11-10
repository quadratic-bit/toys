#pragma once
#include "widgets/objview.hpp"
#include "widgets/toolbox.hpp"
#include "widgets/objlist.hpp"
#include <cstdio>

class Desktop final : public Widget {
public:
    Desktop(Rect2f f, Widget *p, State *s)
            : Widget(f, p, s) {
        Renderer *renderer = new Renderer({185, 50, 800, 600}, NULL, state);
        ControlPanel *toolbox = new ControlPanel(renderer, {5, 25, 175, 150}, NULL, state);
        ObjectView *objview = new ObjectView({5, 200, 175, 175}, NULL, state);
        ObjectsList *objlist = new ObjectsList(renderer->getScene().objects, objview, {1000, 200, 250, 200}, {250, 175}, NULL, state);

        Widget *arr[] = { toolbox, objview, objlist, renderer };
        for (Widget *w : arr) {
            this->appendChild(w);
        }
        this->parent = this;
    }

    const char *title() const override {
        return "Desktop";
    }

    void draw() override {
        texture->Clear(dr4::Color(CLR_BACKGROUND, 255));
    }
};
