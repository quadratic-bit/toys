#include <swuix/widgets/button.hpp>
#include <swuix/state.hpp>

DispatchResult Button::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *) {
    bool c = containsMouse(ctx);
    bool not_covered_or_doesnt_contain = state->mouse.target || !c;
    if (hovered && not_covered_or_doesnt_contain) {
        hovered = false;
        requestRedraw();
    }
    if (c && !state->mouse.target) {
        hovered = true;
        state->mouse.target = this;
        requestRedraw();
    }
    return PROPAGATE;
}

DispatchResult Button::onMouseDown(DispatcherCtx, const MouseDownEvent *) {
    if (state->mouse.target != this) return PROPAGATE;
    pressed = true;
    requestRedraw();
    state->mouse.capture = this;
    if (action) action->apply(state, this);

    return CONSUME;
}

DispatchResult Button::onMouseUp(DispatcherCtx, const MouseUpEvent *) {
    if (pressed) {
        pressed = false;
        requestRedraw();
    }
    if (state->mouse.capture == this) {
        state->mouse.capture = nullptr;
        return CONSUME;
    };
    return PROPAGATE;
}
