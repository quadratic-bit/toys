#include <swuix/widgets/draggable.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

const float SCROLL_DELTA_PX = 20.0f;

class ScrollUp : public Action {
    ScrollableWidget *owner;

public:
    ScrollUp(ScrollableWidget *w) : owner(w) {}

    void apply(void *, Widget *) {
        owner->scroll_y(SCROLL_DELTA_PX);
    }
};

class ScrollDown : public Action {
    ScrollableWidget *owner;

public:
    ScrollDown(ScrollableWidget *w) : owner(w) {}

    void apply(void *, Widget *) {
        owner->scroll_y(-SCROLL_DELTA_PX);
    }
};

ScrollbarSlider::ScrollbarSlider(Rect2F f, Scrollbar *par, State *st)
    : Widget(f, par, st), DraggableWidget(f, par, st), scrollbar(par) {}

void ScrollbarSlider::render(Window *window, float off_x, float off_y) {
    window->clear_rect(frame, off_x, off_y, CLR_PLATINUM);
    window->outline(frame, off_x, off_y, 1);
}

DispatchResult ScrollbarSlider::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
    (void)e;
    if (state->mouse.state == MouseState::Dragging && is_dragging) {
        float progress_px = std::min(
            std::max(ctx.mouseRel.y - start_drag_y + SCROLL_B_H, SCROLL_B_H),
            parent->frame.h - SCROLL_B_H - frame.h
        ) - SCROLL_B_H;
        float progress_per = progress_px / scrollbar->scroll_height();
        float offset_parent_px = scrollbar->host->frame.h * progress_per;
        scrollbar->host->frame.y = scrollbar->host->viewport.y - offset_parent_px;
        scrollbar->host->refresh_layout();
    }
    return Widget::on_mouse_move(ctx, e);
}

DispatchResult ScrollbarSlider::on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e) {
    (void)e;
    if (state->mouse.target == this) {
        is_dragging = true;
        start_drag_x = ctx.mouseRel.x;
        start_drag_y = ctx.mouseRel.y - scrollbar->scroll_progress();
        return CONSUME;
    }
    return PROPAGATE;
}

Scrollbar::Scrollbar(State *state_)
        : Widget(scrollbar_box_zero(), NULL, state_),
        Control(scrollbar_box_zero(), NULL, state_),
        WidgetContainer(scrollbar_box_zero(), NULL, state_), host(NULL) {
    static float h = SCROLL_B_H;

    slider = new ScrollbarSlider(frect(0, h, SCROLLBAR_W, 0), this, state);

    Widget *btns[] = { slider };
    this->append_children(makeChildren(btns));
}

void Scrollbar::attach_to(ControlledWidget *host_) {
    ScrollableWidget *scrollable = dynamic_cast<ScrollableWidget*>(host_);
    if (!scrollable) throw new TraitCastError("host_ must be scrollable");
    host = scrollable;
    host_->attach(this);

    frame = scrollbar_box(host_->frame);

    static float h = SCROLL_B_H;
    Button *btn_up   = new Button(frect(0, 0,           SCROLLBAR_W, h), this, "ʌ", state, new ScrollUp  (host));
    Button *btn_down = new Button(frect(0, frame.h - h, SCROLLBAR_W, h), this, "v", state, new ScrollDown(host));

    Widget *btns[] = { btn_up, btn_down };
    this->append_children(makeChildren(btns));
}

float Scrollbar::scroll_progress() {
    return host->content_progress() / host->frame.h * scroll_height();
}
