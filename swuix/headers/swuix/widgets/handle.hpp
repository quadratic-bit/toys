#pragma once
#include <swuix/widgets/controlled.hpp>
#include <swuix/widgets/draggable.hpp>
#include <swuix/widgets/container.hpp>
#include <swuix/widgets/button.hpp>

const float HANDLE_H = 20.0f;

static inline const FRect handle_box(FRect parent_box) {
	FRect box;
	box.x = 0.0f;
	box.y = -HANDLE_H;
	box.w = parent_box.w;
	box.h = HANDLE_H;
	return box;
}

static inline const FRect handle_box_zero() {
	FRect box;
	box.x = 0.0f;
	box.y = -HANDLE_H;
	box.w = 0.0f;
	box.h = HANDLE_H;
	return box;
}

class MinimizableWidget; // forward-declare

class TitleBar : public Control, public DraggableWidget, public WidgetContainer {
	Button *btn_minimize;
	MinimizableWidget *host;

public:
	TitleBar(State *state_);

	DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);

	DispatchResult on_layout(DispatcherCtx, const LayoutEvent *);

	void attach_to(ControlledWidget *host_);

	const char *title() const {
		return "Title bar";
	}

	void render(Window *window, float off_x, float off_y);
};

class MinimizableWidget : public virtual Widget {
public:
	bool minimized;
	MinimizableWidget(FRect dim_, Widget *parent_, State *state_)
			: Widget(dim_, parent_, state_), minimized(false) {}

	DispatchResult on_render(DispatcherCtx ctx, const RenderEvent *e) {
		if (minimized) return PROPAGATE;
		return Widget::on_render(ctx, e);
	}

	DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
		if (minimized) return PROPAGATE;
		return Widget::broadcast(ctx, e, reversed);
	}
};

class TitledWidget : public MinimizableWidget, public ControlledWidget {
	TitleBar *titlebar;

public:
	TitledWidget(FRect content_frame_, Widget *parent_, State *state_)
			: Widget(content_frame_, parent_, state_),
			MinimizableWidget(content_frame_, parent_, state_),
			ControlledWidget(content_frame_, parent_, state_) {
		titlebar = new TitleBar(state_);
		titlebar->attach_to(this);
	}

	DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
		if (!minimized) {
			return ControlledWidget::broadcast(ctx, e, reversed);
		}

		DispatcherCtx local_ctx = ctx.with_offset(frame);

		if (reversed) {
			for (int i = (int)controls.size() - 1; i >= 0; --i)
				if (controls[i]->broadcast(local_ctx, e, true) == CONSUME) return CONSUME;
			return PROPAGATE;
		} else {
			for (size_t i = 0; i < controls.size(); ++i)
				if (controls[i]->broadcast(local_ctx, e) == CONSUME) return CONSUME;
			return PROPAGATE;
		}
	}

	const char *title() const {
		return "Titled widget";
	}
};

class TitledContainer : public MinimizableWidget, public ControlledContainer {
	TitleBar *titlebar;

public:
	TitledContainer(FRect content_frame_, Widget *parent_, State *state_)
			: Widget(content_frame_, parent_, state_),
			MinimizableWidget(content_frame_, parent_, state_),
			ControlledContainer(content_frame_, parent_, state_) {
		titlebar = new TitleBar(state_);
		titlebar->attach_to(this);
	}

	DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
		if (!minimized) {
			return ControlledContainer::broadcast(ctx, e, reversed);
		}

		DispatcherCtx local_ctx = ctx.with_offset(frame);

		if (reversed) {
			for (int i = (int)controls.size() - 1; i >= 0; --i)
				if (controls[i]->broadcast(local_ctx, e, true) == CONSUME) return CONSUME;
			return PROPAGATE;
		} else {
			for (size_t i = 0; i < controls.size(); ++i)
				if (controls[i]->broadcast(local_ctx, e) == CONSUME) return CONSUME;
			return PROPAGATE;
		}
	}

	const char *title() const {
		return "Titled container";
	}
};
