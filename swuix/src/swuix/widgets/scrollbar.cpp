#include "dr4/math/color.hpp"
#include <cstdio>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/widgets/button.hpp>
#include <swuix/state.hpp>

const float SCROLL_DELTA_PX = 20.0f;

static inline const Rect2f scrollbarBoxZero() {
    return {-SCROLLBAR_W, 0, SCROLLBAR_W, 0};
}

static inline const Rect2f scrollbarBox(Vec2f parent_box) {
    return {parent_box.x - SCROLLBAR_W, 0, SCROLLBAR_W, parent_box.y};
}

class ScrollUp : public Action {
    ScrollableWidget *owner;

public:
    ScrollUp(ScrollableWidget *w) : owner(w) {}

    void apply(void*, Widget*) {
        owner->scrollY(SCROLL_DELTA_PX);
    }
};

class ScrollDown : public Action {
    ScrollableWidget *owner;

public:
    ScrollDown(ScrollableWidget *w) : owner(w) {}

    void apply(void*, Widget*) {
        owner->scrollY(-SCROLL_DELTA_PX);
    }
};

VScrollbarSlider::VScrollbarSlider(Rect2f f, VScrollbar *p, State *s)
        : Widget(f, p, s), DraggableWidget(f, p ,s), scrollbar(p) {}

void VScrollbarSlider::draw() {
    texture->Clear(Color(CLR_BORDER_SUBTLE, 255));

    Rect2f f = frame();
    Rectangle r{
        Rect2f(1, 1, f.size.x - 2, f.size.y - 2),
        Color(CLR_BORDER, 225)
    };
    texture->Draw(r); }

// TODO: review
DispatchResult VScrollbarSlider::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) {
    if (state->mouse.state == Mouse::State::Dragging && is_dragging) {
        float progress_px = std::min(
            std::max(ctx.mouse_rel.y - start_drag_y + SCROLL_BUT_H, SCROLL_BUT_H),
            parent->texture->GetHeight() - SCROLL_BUT_H - texture->GetHeight()
        ) - SCROLL_BUT_H;
        float progress_per = progress_px / scrollbar->scrollHeight();
        float offset_parent_px = scrollbar->host->texture->GetHeight() * progress_per;
        scrollbar->host->position.y = scrollbar->host->viewport_pos.y - offset_parent_px;
        scrollbar->host->requestLayout();
        requestRedraw();
    }
    return Widget::onMouseMove(ctx, e);
}

DispatchResult VScrollbarSlider::onMouseDown(DispatcherCtx ctx, const MouseDownEvent *) {
    if (state->mouse.target == this) {
        is_dragging = true;
        start_drag_x = ctx.mouse_rel.x;
        start_drag_y = ctx.mouse_rel.y - scrollbar->scrollProgress();
        return CONSUME;
    }
    return PROPAGATE;
}

VScrollbar::VScrollbar(State *state_) : Widget(scrollbarBoxZero(), NULL, state_) {
    static float h = SCROLL_BUT_H;

    slider = new VScrollbarSlider({0, h, SCROLLBAR_W, 0}, this, state);
    this->appendChild(slider);
}

void VScrollbar::attachTo(ScrollableWidget *scrollable) {
    host = scrollable;
    scrollable->appendChild(this);

    Rect2f box = scrollbarBox(scrollable->texture->GetSize());
    position = box.pos;
    texture->SetSize(box.size);

    static float h = SCROLL_BUT_H;
    const float frameh = texture->GetHeight();
    Button *btn_up   = new Button({0, 0,          SCROLLBAR_W, h}, this, "ʌ", state, new ScrollUp  (host));
    Button *btn_down = new Button({0, frameh - h, SCROLLBAR_W, h}, this, "v", state, new ScrollDown(host));

    this->appendChild(btn_up);
    this->appendChild(btn_down);

    host->requestLayout();
    host->requestRedraw();
}

float VScrollbar::scrollProgress() const {
    return host->contentProgressY() / host->texture->GetHeight() * scrollHeight();
}

DispatchResult ScrollableWidget::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *) {
    bool c = containsMouse(ctx);
    if (!state->mouse.target && c) state->mouse.target = this;
    if (c) state->mouse.wheel_target = this;
    return PROPAGATE;
}

DispatchResult ScrollableWidget::onMouseWheel(DispatcherCtx, const MouseWheelEvent *e) {
    (void)e->delta.x;
    if (e->delta.y) scrollY(e->delta.y * SCROLL_DELTA_PX);
    return CONSUME;
}
