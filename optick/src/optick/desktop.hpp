#pragma once
#include "widgets/canvas.hpp"
#include "widgets/objview.hpp"
#include "widgets/toolbox.hpp"
#include "widgets/objlist.hpp"
#include "widgets/menu_bar.hpp"

class Desktop final : public Widget {
public:
    Desktop(Rect2f f, Widget *p, State *s) : Widget(f, p, s) {
        Renderer *renderer = new Renderer({185, 50, 800, 600}, NULL, state);
        ControlPanel *toolbox = new ControlPanel(renderer, {5, 50, 175, 150}, NULL, state);
        ObjectsList *objlist = new ObjectsList(renderer->getScene().objects, this, {1000, 225, 250, 200}, {250, 175}, NULL, state);
        MenuBar *menu = new MenuBar({0, 0, f.size.x, 25}, NULL, state, this);

        Canvas *c = new Canvas({0, 0, renderer->texture->GetWidth(), renderer->texture->GetHeight()}, NULL, state);
        renderer->appendChild(c);

        Widget *arr[] = { toolbox, objlist, renderer, menu };
        for (Widget *w : arr) {
            this->appendChild(w);
        }
        this->parent = this;
    }

    template<class T>
    T *findChild() {
        for (Widget *child : children) {
            T *v = dynamic_cast<T*>(child);
            if (v) return v;
        }
        return nullptr;
    }

    bool createView() {
        if (findChild<ObjectView>()) return false;
        ObjectView *objview = new ObjectView({5, 225, 175, 175}, NULL, state);
        prependChild(objview);
        return true;
    }

    const char *title() const override {
        return "Desktop";
    }

    void draw() override {
        texture->Clear(dr4::Color(CLR_BACKGROUND, 255));
    }
};
