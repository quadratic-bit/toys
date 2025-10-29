#pragma once
#include "widgets/toolbox.hpp"
#include "widgets/objpreview.hpp"

class Desktop : public WidgetContainer {
public:
    Desktop(Rect2F frame_, Widget *parent_, State *state_)
            : Widget(frame_, parent_, state_), WidgetContainer(frame_, parent_, state_) {
        Renderer *renderer = new Renderer(frect(185, 50, 800, 600), NULL, state);
        ControlPanel *toolbox = new ControlPanel(renderer, frect(5, 25, 175, 150), NULL, state);
        ObjectsList *objview = new ObjectsList(renderer->getScene().objects, frect(1000, 200, 250, 200), frect(1000, 200, 250, 175), NULL, state);

        Widget *arr[] = { toolbox, objview, renderer };
        this->append_children(Widget::makeChildren(arr));
        this->parent = this;
    }

    const char *title() const {
        return "Desktop";
    }

    void render(Window *, float, float) {}
};
