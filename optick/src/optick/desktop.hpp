#pragma once
#include "widgets/objview.hpp"
#include "widgets/toolbox.hpp"
#include "widgets/objlist.hpp"
#include "widgets/menu_bar.hpp"

#include "cum/manager.hpp"

class Desktop final : public Widget {
    cum::Manager *mgr_;

    Renderer     *renderer_ = nullptr;
    ControlPanel *toolbox_  = nullptr;
    ObjectsList  *objlist_  = nullptr;
    MenuBar      *menu_     = nullptr;
    ObjectView   *objview_  = nullptr;

public:
    Desktop(Rect2f f, Widget *p, State *s, cum::Manager *mgr) : Widget(f, p, s), mgr_(mgr) {
        renderer_ = new Renderer({185, 50, 800, 600}, NULL, state, mgr);
        toolbox_  = new ControlPanel(renderer_, {5, 50, 175, 150}, NULL, state);
        objlist_  = new ObjectsList(renderer_->getScene().objects, this, {1000, 225, 250, 200}, {250, 175}, NULL, state);
        menu_     = new MenuBar({0, 0, f.size.x, 25}, NULL, state, this);

        Widget *arr[] = { toolbox_, objlist_, renderer_, menu_ };
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
        if (objview_ || findChild<ObjectView>()) return false;
        objview_ = new ObjectView({5, 225, 175, 225}, NULL, state);
        prependChild(objview_);
        return true;
    }

    void togglePropertiesView() {
        if (objview_) {
            destroyChild(objview_);
            objview_ = nullptr;
            requestRedraw();
            return;
        }
        createView();
        requestRedraw();
    }

    void toggleObjectsList() {
        if (objlist_) {
            destroyChild(objlist_);
            objlist_ = nullptr;
            requestRedraw();
            return;
        }
        if (!renderer_) return;

        objlist_ = new ObjectsList(
            renderer_->getScene().objects,
            this,
            {1000, 225, 250, 200},
            {250, 175},
            NULL,
            state
        );
        appendChild(objlist_);
        requestRedraw();
    }

    void toggleControlsPanel() {
        if (toolbox_) {
            destroyChild(toolbox_);
            toolbox_ = nullptr;
            requestRedraw();
            return;
        }
        if (!renderer_) return;

        toolbox_ = new ControlPanel(renderer_, {5, 50, 175, 150}, NULL, state);
        appendChild(toolbox_);
        requestRedraw();
    }

    void toggleRendererPanel() {
        if (renderer_) {
            // dependent panels rely on renderer_
            if (toolbox_) { destroyChild(toolbox_); toolbox_ = nullptr; }
            if (objlist_) { destroyChild(objlist_); objlist_ = nullptr; }
            if (objview_) { destroyChild(objview_); objview_ = nullptr; }

            destroyChild(renderer_);
            renderer_ = nullptr;

            requestRedraw();
            return;
        }

        renderer_ = new Renderer({185, 50, 800, 600}, NULL, state, mgr_);
        appendChild(renderer_);

        appendChild(toolbox_);
        appendChild(objlist_);

        requestRedraw();
    }

    const char *title() const override {
        return "Desktop";
    }

    void draw() override {
        texture->Clear(dr4::Color(CLR_BACKGROUND, 255));
    }
};
