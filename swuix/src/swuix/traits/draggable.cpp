#include <swuix/traits/draggable.hpp>
#include <swuix/state.hpp>

DispatchResult DraggableWidget::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) {
    if (state->mouse.state == Mouse::State::Dragging && is_dragging) {
        translate({ctx.mouse_rel.x - start_drag_x, ctx.mouse_rel.y - start_drag_y});
        parent->requestRedraw();
    }
    return Widget::onMouseMove(ctx, e);
}

DispatchResult DraggableWidget::onMouseDown(DispatcherCtx ctx, const MouseDownEvent *) {
    if (state->mouse.target == this) {
        is_dragging = true;
        start_drag_x = ctx.mouse_rel.x;
        start_drag_y = ctx.mouse_rel.y;
        return CONSUME;
    }
    return PROPAGATE;
}

DispatchResult DraggableWidget::onMouseUp(DispatcherCtx, const MouseUpEvent *) {
    if (is_dragging) is_dragging = false;
    return PROPAGATE;
}
