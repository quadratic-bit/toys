#include <cstdio>
#include <swuix/widgets/draggable.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

class ScrollUp : public Action {
	ScrollableWidget *owner;
public:
	ScrollUp(ScrollableWidget *w) : owner(w) {}

	void apply(void *, Widget *) {
		owner->scroll_up();
	}
};

class ScrollDown : public Action {
	ScrollableWidget *owner;
public:
	ScrollDown(ScrollableWidget *w) : owner(w) {}

	void apply(void *, Widget *) {
		owner->scroll_down();
	}
};

ScrollbarSlider::ScrollbarSlider(FRect f, Scrollbar *par, State *st)
		: Widget(f, par, st), DraggableWidget(f, par, st), scrollbar(par) {}

void ScrollbarSlider::render(Window *window, float off_x, float off_y) {
	window->clear_rect(frame, off_x, off_y, CLR_PLATINUM);
	window->outline(frame, off_x, off_y, 1);
}

DispatchResult ScrollbarSlider::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (state->mouse.state == MouseState::Dragging && is_dragging) {
		float progress_px = std::min(
			std::max(ctx.mouse_rel.y - start_drag_y + SCROLL_B_H, SCROLL_B_H),
			parent->frame.h - SCROLL_B_H - frame.h
		) - SCROLL_B_H;
		float progress_per = progress_px / (scrollbar->frame.h - 2 * SCROLL_B_H);
		float offset_parent_px = scrollbar->owner->frame.h * progress_per;
		scrollbar->owner->frame.y = scrollbar->owner->viewport.y - offset_parent_px;
		scrollbar->owner->layout();
	}
        return Widget::on_mouse_move(ctx, e);
}

Scrollbar::Scrollbar(ScrollableWidget *parent_, State *state_)
		: Widget(scrollbar_box(parent_->viewport), parent_, state_),
		WidgetContainer(scrollbar_box(parent_->viewport), parent_, state_),
		owner(parent_) {
	static float h = SCROLL_B_H;
	Button *btn_up   = new Button(frect(0, 0,           SCROLLBAR_W, h), this, "ʌ", state, new ScrollUp  (parent_));
	Button *btn_down = new Button(frect(0, frame.h - h, SCROLLBAR_W, h), this, "v", state, new ScrollDown(parent_));
	float slider_h = (frame.h - 2 * h) * (parent_->viewport.h / parent_->frame.h);
	ScrollbarSlider *slider = new ScrollbarSlider(frect(0, h, SCROLLBAR_W, slider_h), this, state);
	Widget *btns[] = { slider, btn_up, btn_down };
	this->append_children(make_children(btns));
}
