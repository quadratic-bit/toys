#pragma once
#include "widgets/objview.hpp"
#include "widgets/toolbox.hpp"
#include "widgets/objlist.hpp"
#include "widgets/menu_bar.hpp"

#include "cum/manager.hpp"

class Desktop final : public Widget {
    cum::Manager *mgr_;

public:
    Desktop(Rect2f f, Widget *p, State *s, cum::Manager *mgr) : Widget(f, p, s), mgr_(mgr) {
        Renderer *renderer = new Renderer({185, 50, 800, 600}, NULL, state, mgr);
        ControlPanel *toolbox = new ControlPanel(renderer, {5, 50, 175, 150}, NULL, state);
        ObjectsList *objlist = new ObjectsList(renderer->getScene().objects, this, {1000, 225, 250, 200}, {250, 175}, NULL, state);
        MenuBar *menu = new MenuBar({0, 0, f.size.x, 25}, NULL, state, this);

        Widget *arr[] = { toolbox, objlist, renderer, menu };
        for (Widget *w : arr) {
            this->appendChild(w);
        }
        this->parent = this;
    }

    cum::Manager *manager() const { return mgr_; }

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
        ObjectView *objview = new ObjectView({5, 225, 175, 225}, NULL, state);
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
