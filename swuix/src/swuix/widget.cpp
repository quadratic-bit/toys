#include <swuix/widget.hpp>
#include <swuix/state.hpp>

Widget::~Widget() {
	if (state->mouse.target == this) {
		state->mouse.target = NULL;
	}
	if (state->mouse.capture == this) {
		state->mouse.capture = NULL;
	}
}

DispatchResult Widget::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (!state->mouse.target && contains_mouse(ctx)) {
		state->mouse.target = this;
	}
	return PROPAGATE;
}

static DispatcherCtx _build_context(const Widget *w, const Point2f &abs, Window *window) {
	if (w->parent == w) {
		return DispatcherCtx::from_absolute(abs, w->frame, window);
	}

	// ctx is currently in w->parent's coordinate space
	DispatcherCtx ctx = _build_context(w->parent, abs, window);

	ctx.clip(w->parent->get_viewport());

	ctx = ctx.with_offset(w->parent->frame);
	return ctx;
}

DispatcherCtx Widget::resolve_context(Window *w) const {
	DispatcherCtx ctx = _build_context(this, state->mouse.pos, w);
	ctx.clip(this->get_viewport());
	return ctx;
}
