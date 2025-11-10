#include <swuix/traits/focusable.hpp>
#include <swuix/state.hpp>

DispatchResult FocusableWidget::onMouseDown(DispatcherCtx ctx, const MouseDownEvent *) {
    if (containsMouse(ctx)) state->focus(this);
    return PROPAGATE;
}
