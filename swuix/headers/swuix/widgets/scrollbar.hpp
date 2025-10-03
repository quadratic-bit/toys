#pragma once
#include <swuix/widgets/container.hpp>
#include <swuix/widgets/button.hpp>

const float SCROLLBAR_W     = 10.0f;
const float SCROLL_DELTA_PX = 20.0f;

static inline const FRect scrollbar_box(FRect parent_box) {
	FRect box;
	box.x = parent_box.w - SCROLLBAR_W;
	box.y = 0;
	box.w = SCROLLBAR_W;
	box.h = parent_box.h;
	return box;
}

// forward-declare
class ScrollableWidget;

class Scrollbar : public WidgetContainer {
public:
	Scrollbar(ScrollableWidget *parent_, State *state_);

	const char *title() const {
		return "Scrollbar";
	}

	void render(Window *window, int off_x, int off_y) {
		window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
		WidgetContainer::render(window, off_x, off_y);
		window->outline(frame, off_x, off_y, 2);
	}
};

class ScrollableWidget : public virtual Widget {
	Scrollbar *scrollbar;
public:
	FRect viewport;

	ScrollableWidget(FRect content_frame_, FRect viewport_frame_, Widget *parent_, State *state_)
			: Widget(content_frame_, parent_, state_), viewport(viewport_frame_) {
		scrollbar = new Scrollbar(this, state_);
	}

	virtual void render_body(Window *window, int off_x, int off_y) = 0;

	void render(Window *window, int off_x, int off_y) {
		window->clip(viewport);
		render_body(window, off_x, off_y);
		scrollbar->render(window, frame.x + off_x, frame.y + off_y);
		window->unclip();
	}

	Scrollbar *scrollbar_widget() {
		return scrollbar;
	}

	void scroll_up() {
		float available = viewport.y - frame.y;
		if (available <= 0) return;
		if (available <= SCROLL_DELTA_PX) frame.y = viewport.y;
		else frame.y += SCROLL_DELTA_PX;
		scrollbar->frame.y = viewport.y - frame.y;
	}

	void scroll_down() {
		float available = (frame.y + frame.h) - (viewport.y + viewport.h);
		if (available <= 0) return;
		if (available <= SCROLL_DELTA_PX) frame.y = viewport.y + viewport.h - frame.h;
		else frame.y -= SCROLL_DELTA_PX;
		scrollbar->frame.y = viewport.y - frame.y;
	}

	size_t child_count() const {
		return 1;
	}

	Widget *child_at(size_t i) const {
		assert(i == 0);
		return scrollbar;
	}
};

class ScrollableContainer : public ScrollableWidget, public WidgetContainer {
public:
	ScrollableContainer(FRect content_frame_, FRect viewport_frame_, Widget *parent_, State *state_)
		: Widget(content_frame_, parent_, state_),
		ScrollableWidget(content_frame_, viewport_frame_, parent_, state_),
		WidgetContainer(content_frame_, parent_, state_) {}

	ScrollableContainer(FRect content_frame_, FRect viewport_frame_, Widget *parent_, std::vector<Widget*> children_, State *state_)
		: Widget(content_frame_, parent_, state_),
		ScrollableWidget(content_frame_, viewport_frame_, parent_, state_),
		WidgetContainer(content_frame_, parent_, children_, state_) {}

	const char *title() const {
		return "ScrollableContainer";
	}

	size_t child_count() const {
		return ScrollableWidget::child_count() + WidgetContainer::child_count();
	}

	Widget *child_at(size_t i) const {
		if (i == child_count() - 1) {
			return ScrollableWidget::child_at(0);
		}
		return WidgetContainer::child_at(i);
	}

	void render(Window *window, int off_x, int off_y) {
		ScrollableWidget::render(window, off_x, off_y);
	}

	void render_body(Window *window, int off_x, int off_y) {
		WidgetContainer::render(window, off_x, off_y);
	}
};
