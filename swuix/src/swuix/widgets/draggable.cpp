#include <swuix/widgets/draggable.hpp>
#include <swuix/state.hpp>

DispatchResult DraggableWidget::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
    (void)e;
    if (state->mouse.state == MouseState::Dragging && is_dragging) {
        Rect2F new_frame = frame;
        new_frame.x = ctx.mouseRel.x - start_drag_x;
        new_frame.y = ctx.mouseRel.y - start_drag_y;
        set_frame(new_frame);
    }
    return Widget::on_mouse_move(ctx, e);
}

DispatchResult DraggableWidget::on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e) {
    (void)e;
    if (state->mouse.target == this) {
        is_dragging = true;
        start_drag_x = ctx.mouseRel.x;
        start_drag_y = ctx.mouseRel.y;
        return CONSUME;
    }
    return PROPAGATE;
}

DispatchResult DraggableWidget::on_mouse_up(DispatcherCtx ctx, const MouseUpEvent *e) {
    (void)e;
    (void)ctx;
    if (is_dragging) is_dragging = false;
    return PROPAGATE;
}
