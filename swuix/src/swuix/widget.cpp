#include <swuix/widget.hpp>
#include <swuix/state.hpp>

Widget::~Widget() {
	size_t n = child_count();
	for (size_t i = 0; i < n; ++i) {
		delete child_at(i);
	}
	if (state->mouse.target == this) {
		state->mouse.target = NULL;
	}
	if (state->mouse.capture == this) {
		state->mouse.capture = NULL;
	}
}

DispatchResult Widget::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (!state->mouse.target && contains_point(ctx)) {
		state->mouse.target = this;
	}
	return PROPAGATE;
}

static DispatcherCtx _build_capture_context(const Widget *w, const Point2f &abs) {
	if (w->parent == w) {
		return DispatcherCtx::from_absolute(abs, w->frame);
	}

	// ctx is currently in w->parent's coordinate space
	DispatcherCtx ctx = _build_capture_context(w->parent, abs);

	ctx.clip(w->parent->get_viewport());

	ctx = ctx.with_offset(w->parent->frame);
	return ctx;
}

DispatcherCtx Widget::resolve_capture_context() const {
	DispatcherCtx ctx = _build_capture_context(this, state->mouse.pos);
	ctx.clip(this->get_viewport());
	return ctx;
}
