#pragma once
#include "draggable.hpp"
#include "container.hpp"
#include "button.hpp"

const float HANDLE_H = 20.0f;

static inline const SDL_FRect handle_box(SDL_FRect parent_box) {
	SDL_FRect box;
	box.x = 0;
	box.y = -HANDLE_H;
	box.w = parent_box.w;
	box.h = HANDLE_H;
	return box;
}

static void cb_minimize(State *st, Widget *w);

class Handle : public DraggableWidget, public WidgetContainer {
public:
	Handle(Widget *parent_, State *state_)
			: Widget(handle_box(parent_->frame), parent_, state_),
			DraggableWidget(handle_box(parent_->frame), parent_, state_),
			WidgetContainer(handle_box(parent_->frame), parent_, state_) {
		static float w = 15, h = 10;
		Button *btn_minimize = new Button(frect(frame.w - w - 5, 5, w, h), this, "-", state, cb_minimize);
		Widget *btns[] = { btn_minimize };
		this->append_children(make_children(btns));
	}

	DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);

	void adjust_width(float new_width);

	void render(Window *window, int off_x, int off_y);
};

class HandledWidget : public virtual Widget {
	Handle *handle;

	DispatchResult route_minimized(DispatcherCtx ctx, Event *e) {
		DispatcherCtx here = ctx.with_offset(frame.x, frame.y);

		// TODO: generalize
		if (e->is_pointer()) {
			DispatchResult r = handle->route(here, e);
			if (r == CONSUME) {
				return CONSUME;
			}
		} else {
			if (handle->route(here, e) == CONSUME) {
				return CONSUME;
			}
		}

		return PROPAGATE;
	}

public:
	bool minimized;
	HandledWidget(SDL_FRect dim_, Widget *parent_, State *state_)
		: Widget(dim_, parent_, state_), handle(new Handle(this, state_)), minimized(false) {}

	virtual void render_body(Window *window, int off_x, int off_y) = 0;

	void render(Window *window, int off_x, int off_y);

	Handle *handle_widget() {
		return handle;
	}
	
	size_t child_count() const {
		return 1;
	}

	Widget *child_at(size_t i) const {
		assert(i == 0);
		return handle;
	}

	DispatchResult route(DispatcherCtx ctx, Event *e) {
		if (minimized) return route_minimized(ctx, e);
		return Widget::route(ctx, e);
	}
};

static void cb_minimize(State *st, Widget *w) {
	(void)st;
	HandledWidget *par = dynamic_cast<HandledWidget*>(w->parent->parent);
	par->minimized ^= true;
}

