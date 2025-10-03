#pragma once
#include <swuix/widgets/handle.hpp>
#include <swuix/widgets/container.hpp>
#include <swuix/widgets/button.hpp>

const float SCROLLBAR_W     = 10.0f;
const float SCROLL_B_H      = 10.0f;
const float SCROLL_DELTA_PX = 20.0f;

static inline const FRect scrollbar_box(FRect parent_box) {
	FRect box;
	box.x = parent_box.w - SCROLLBAR_W;
	box.y = 0;
	box.w = SCROLLBAR_W;
	box.h = parent_box.h;
	return box;
}

class Scrollbar;  // forward-declare

class ScrollbarSlider : public DraggableWidget {
	Scrollbar *scrollbar;

public:
	ScrollbarSlider(FRect f, Scrollbar *par, State *st);

	const char *title() const {
		return "Scrollbar Slider";
	}

	DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);
	DispatchResult on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e);

	void render(Window *window, float off_x, float off_y);
};

class ScrollableWidget;  // forward-declare

class Scrollbar : public WidgetContainer {
public:
	ScrollableWidget *owner;

	Scrollbar(ScrollableWidget *parent_, State *state_);

	const char *title() const {
		return "Scrollbar";
	}

	void render(Window *window, float off_x, float off_y) {
		window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
		WidgetContainer::render(window, off_x, off_y);
		window->outline(frame, off_x, off_y, 2);
	}
};

class ScrollableWidget : public virtual Widget {
protected:
	Scrollbar *scrollbar;

public:
	FRect viewport;

	ScrollableWidget(FRect content_frame_, FRect viewport_frame_, Widget *parent_, State *state_)
			: Widget(content_frame_, parent_, state_), viewport(viewport_frame_) {
		scrollbar = new Scrollbar(this, state_);
	}

	virtual void render_body(Window *window, float off_x, float off_y) = 0;

	void set_frame(FRect new_frame) {
		float ydiff = viewport.y - frame.y;
		float xdiff = viewport.x - frame.x;
		frame = new_frame;
		viewport.x = frame.x + xdiff;
		viewport.y = frame.y + ydiff;
		layout();
	}

	void render(Window *window, float off_x, float off_y) {
		render_body(window, off_x, off_y);
		scrollbar->render(window, frame.x + off_x, frame.y + off_y);
	}

	Scrollbar *scrollbar_widget() {
		return scrollbar;
	}

	bool contains_point(Point2f rel) const {
		bool res = rel.x >= viewport.x && rel.x <= viewport.x + viewport.w
			&& rel.y >= viewport.y && rel.y <= viewport.y + viewport.h;
		return res;
	}

	void scroll_up() {
		float available = viewport.y - frame.y;
		if (available <= 0) return;
		if (available <= SCROLL_DELTA_PX) frame.y = viewport.y;
		else frame.y += SCROLL_DELTA_PX;
		layout();
	}

	void scroll_down() {
		float available = (frame.y + frame.h) - (viewport.y + viewport.h);
		if (available <= 0) return;
		if (available <= SCROLL_DELTA_PX) frame.y = viewport.y + viewport.h - frame.h;
		else frame.y -= SCROLL_DELTA_PX;
		layout();
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

	void render(Window *window, float off_x, float off_y) {
		ScrollableWidget::render(window, off_x, off_y);
	}

	void render_body(Window *window, float off_x, float off_y) {
		WidgetContainer::render(window, off_x, off_y);
	}
};

class TallView : public HandledWidget, public ScrollableContainer {
public:
	TallView(FRect content_frame_, FRect viewport_frame_, Widget *parent_, State *state_)
		: Widget(content_frame_, parent_, state_),
		HandledWidget(content_frame_, parent_, state_),
		ScrollableContainer(content_frame_, viewport_frame_, parent_, state_) {}

	const char *title() const {
		return "Tall View";
	}

	void layout() {
		float progress_px = viewport.y - frame.y;
		handle->frame.y = progress_px - HANDLE_H;
		scrollbar->frame.y = viewport.y - frame.y;
		float progress_per = progress_px / frame.h;
		scrollbar->child_at(0)->frame.y = SCROLL_B_H + progress_per * (scrollbar->frame.h - 2 * SCROLL_B_H);
	}

	size_t child_count() const {
		return WidgetContainer::child_count() + 2;
	}

	Widget *child_at(size_t i) const {
		if (i == child_count() - 1) {
			return handle;
		} else if (i == child_count() - 2) {
			return scrollbar;
		}
		return WidgetContainer::child_at(i);
	}

	void render(Window *window, float off_x, float off_y) {
		FRect clip_rect = viewport;
		clip_rect.y -= HANDLE_H;
		clip_rect.h += 2 * HANDLE_H;

		if (!minimized) {
			render_body(window, off_x, off_y);
			scrollbar->render(window, frame.x + off_x, frame.y + off_y);
		}
		handle->render(window, frame.x + off_x, frame.y + off_y);
	}

	void render_body(Window *window, float off_x, float off_y) {
		WidgetContainer::render(window, off_x, off_y);
	}
};
