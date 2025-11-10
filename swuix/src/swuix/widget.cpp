#include <swuix/widget.hpp>
#include <swuix/state.hpp>

Widget::Widget(Rect2f frame, Widget *p, State *s)
        : parent(p ? p : this), position(frame.pos), state(s) {
    texture = state->window->CreateTexture();
    if (frame.size.x == 0) frame.size.x = 1;
    if (frame.size.y == 0) frame.size.y = 1;
    texture->SetSize(frame.size);
}

Widget::~Widget() {
    if (state->mouse.target == this) {
        state->mouse.target = nullptr;
    }

    if (state->mouse.capture == this) {
        state->mouse.capture = nullptr;
    }

    for (Widget *child : children)  {
        delete child;
    }
}

DispatchResult Widget::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) {
    (void)e;
    if (!state->mouse.target && containsMouse(ctx)) {
        state->mouse.target = this;
    }
    return PROPAGATE;
}
