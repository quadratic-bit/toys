#pragma once
#include <cstdio>

#include <swuix/widgets/container.hpp>
#include <swuix/widgets/handle.hpp>

#include "renderer.hpp"

class Desktop : public WidgetContainer {
public:
    Desktop(Rect2F frame_, Widget *parent_, State *state_)
        : Widget(frame_, parent_, state_), WidgetContainer(frame_, parent_, state_) {

            Renderer *renderer = new Renderer(frect(180, 50, 800, 600), NULL, state);

            Widget *arr[] = { renderer };
            this->append_children(Widget::makeChildren(arr));
            this->parent = this;
        }

    const char *title() const {
        return "Desktop";
    }

    void render(Window *, float, float) {}
};
