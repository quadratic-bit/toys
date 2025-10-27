#include <swuix/traits/focusable.hpp>
#include <swuix/state.hpp>

DispatchResult FocusableWidget::on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e) {
    (void)e;
    if (contains_mouse(ctx)) state->mouse.focus = this;
    return PROPAGATE;
}
