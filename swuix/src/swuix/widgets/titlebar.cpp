#include <swuix/widgets/titlebar.hpp>
#include <swuix/state.hpp>

class ToggleMinimize : public Action {
    MinimizableWidget *target;
public:
    ToggleMinimize(MinimizableWidget *w) : target(w) {}

    void apply(void *, Widget *) {
        target->minimized ^= true;
    }
};

TitleBar::TitleBar(State *state_)
    : Widget(handle_box_zero(), NULL, state_),
        Control(handle_box_zero(), NULL, state_),
        DraggableWidget(handle_box_zero(), NULL, state_),
        WidgetContainer(handle_box_zero(), NULL, state_), host(NULL) {
    static float w = 15, h = 10;
    btn_minimize = new Button(frect(frame.w - w - 5, 5, w, h), this, "-", state);
    Widget *btns[] = { btn_minimize };
    this->append_children(makeChildren(btns));
}

DispatchResult TitleBar::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
    (void)e;
    if (state->mouse.state == MouseState::Dragging && is_dragging) {
        Rect2F new_frame = host->frame;
        new_frame.x += ctx.mouseRel.x - start_drag_x;
        new_frame.y += ctx.mouseRel.y - start_drag_y;
        host->set_frame(new_frame);
    }
    return Widget::on_mouse_move(ctx, e);
}

DispatchResult TitleBar::on_layout(DispatcherCtx, const LayoutEvent *) {
    frame.w = host->frame.w;
    btn_minimize->frame.x = frame.w - 20;
    return PROPAGATE;
}

void TitleBar::attach_to(ControlledWidget *host_) {
    MinimizableWidget *minimizable = dynamic_cast<MinimizableWidget*>(host_);
    if (!minimizable) throw new TraitCastError("host_ must be minimizable");
    host = minimizable;
    host_->attach(this);
    btn_minimize->set_action(new ToggleMinimize(host));
    frame = handle_box(host_->frame);
    host_->refresh_layout();
}

void TitleBar::render(Window *window, float off_x, float off_y) {
    window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
    window->outline(frame, off_x, off_y, 2);
    window->text(host->title(), frame.x + off_x + 3, frame.y + off_y + 1);
}
