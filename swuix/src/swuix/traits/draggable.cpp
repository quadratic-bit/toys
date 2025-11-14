#include <swuix/traits/draggable.hpp>
#include <swuix/state.hpp>

DispatchResult DraggableWidget::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) {
    if (state->mouse.state == Mouse::State::Dragging && is_dragging) {
        translate(start_drag_pos + dr4::Vec2f{ctx.mouse_rel - start_drag});
        parent->requestRedraw();
    }
    return Widget::onMouseMove(ctx, e);
}

DispatchResult DraggableWidget::onMouseDown(DispatcherCtx ctx, const MouseDownEvent *) {
    if (state->mouse.target == this) {
        is_dragging = true;
        start_drag = ctx.mouse_rel;
        start_drag_pos = position;
        return CONSUME;
    }
    return PROPAGATE;
}

DispatchResult DraggableWidget::onMouseUp(DispatcherCtx, const MouseUpEvent *) {
    if (is_dragging) is_dragging = false;
    return PROPAGATE;
}
