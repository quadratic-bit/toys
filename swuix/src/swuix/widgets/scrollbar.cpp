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

Scrollbar::Scrollbar(ScrollableWidget *parent_, State *state_)
		: Widget(scrollbar_box(parent_->viewport), parent_, state_),
		WidgetContainer(scrollbar_box(parent_->viewport), parent_, state_) {
	static float h = 10;
	Button *btn_up   = new Button(frect(0, 0,           SCROLLBAR_W, h), this, "^", state, new ScrollUp  (parent_));
	Button *btn_down = new Button(frect(0, frame.h - h, SCROLLBAR_W, h), this, "v", state, new ScrollDown(parent_));
	Widget *btns[] = { btn_up, btn_down };
	this->append_children(make_children(btns));
}
