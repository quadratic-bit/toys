#pragma once
#include <cstdlib>
#include <vector>

#include <swuix/common.hpp>
#include <swuix/window/window.hpp>
#include <swuix/geometry.hpp>

// forward-declare
class Widget;
struct State;

struct DispatcherCtx {
	Point2f mouse_rel;
	Point2f mouse_abs;
	FRect   viewport;
	Window *window;

	void clip(FRect frame) {
		viewport = intersect(viewport, frame);
	}

	Point2f offset() {
		return mouse_abs - mouse_rel;
	}

	DispatcherCtx with_offset(FRect frame) const {
		DispatcherCtx c = *this;
		c.mouse_rel.x -= frame.x;
		c.mouse_rel.y -= frame.y;
		c.viewport.x -= frame.x;
		c.viewport.y -= frame.y;
		return c;
	}

	static DispatcherCtx from_absolute(Point2f abs, FRect clip, Window *window) {
		DispatcherCtx c;
		c.mouse_abs = c.mouse_rel = abs;
		c.viewport = clip;
		c.window = window;
		return c;
	}
};

enum DispatchResult {
	PROPAGATE,
	CONSUME
};

struct Event {
	virtual ~Event() {}
	virtual DispatchResult deliver(DispatcherCtx ctx, Widget *w) = 0;
};

struct RenderEvent : Event {
	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct LayoutEvent : Event {
	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct IdleEvent : public Event {
	Time dt_s;
	Time budget_s;
	Time deadline;
	IdleEvent(Time dt_s_, Time budget_s_, Time deadline_)
		: dt_s(dt_s_), budget_s(budget_s_), deadline(deadline_) {}

	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct QuitRequestEvent : Event {
	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseMoveEvent : Event {
	Point2f mouse_abs;
	MouseMoveEvent(Point2f mouse_abs_) : mouse_abs(mouse_abs_) {}

	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseDownEvent : Event {
	Point2f mouse_abs;
	MouseDownEvent(Point2f mouse_abs_) : mouse_abs(mouse_abs_) {}

	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseUpEvent : Event {
	MouseUpEvent() {}

	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct KeyEvent : Event {
	int scancode;   // platform-neutral
	int keycode;
	uint16_t mods;  // bitmask

	KeyEvent(int scancode_, int keycode_, uint32_t mods_)
		: scancode(scancode_), keycode(keycode_), mods(mods_) {}
};

struct KeyDownEvent : KeyEvent {
	bool repeat;
	KeyDownEvent(int scancode_, int keycode_, uint32_t mods_, bool repeat_)
		: KeyEvent(scancode_, keycode_, mods_), repeat(repeat_) {}

	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct KeyUpEvent : KeyEvent {
	KeyUpEvent(int scancode_, int keycode_, uint32_t mods_)
		: KeyEvent(scancode_, keycode_, mods_) {}

	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

class Widget {
public:
	Widget *parent;
	State  *state;
	FRect   frame;  // `x` and `y` are relative to `parent`

	Widget(FRect frame_, Widget *parent_, State *state_)
		: parent(parent_ ? parent_ : this), state(state_), frame(frame_) {};

	virtual ~Widget();

	template<typename T, size_t N>
	static std::vector<T> make_children(T const (&arr)[N]) {
		return std::vector<T>(arr, arr + N);
	}

	virtual FRect get_viewport() const { return frame; }
	virtual void set_frame(FRect new_frame) { frame = new_frame; }

	virtual const char *title() const = 0;

	virtual bool contains_mouse(DispatcherCtx ctx) const {
		Point2f rel = ctx.mouse_rel;
		FRect view = ctx.viewport;
		return rel.x >= view.x && rel.x <= view.x + view.w
			&& rel.y >= view.y && rel.y <= view.y + view.h;
		// FIXME
		//Point2f rel = ctx.mouse_rel;
		//return rel.x >= frame.x && rel.x <= frame.x + frame.w
		//	&& rel.y >= frame.y && rel.y <= frame.y + frame.h;
	}

	virtual void render(Window *window, float off_x, float off_y) = 0;

	virtual DispatchResult on_render(DispatcherCtx ctx, const RenderEvent *) {
		Point2f off = ctx.offset();
		render(ctx.window, off.x, off.y);

		return PROPAGATE;
	}

	virtual DispatchResult on_mouse_move  (DispatcherCtx, const MouseMoveEvent   *);
	virtual DispatchResult on_mouse_down  (DispatcherCtx, const MouseDownEvent   *) { return PROPAGATE; }
	virtual DispatchResult on_mouse_up    (DispatcherCtx, const MouseUpEvent     *) { return PROPAGATE; }
	virtual DispatchResult on_key_down    (DispatcherCtx, const KeyDownEvent     *) { return PROPAGATE; }
	virtual DispatchResult on_key_up      (DispatcherCtx, const KeyUpEvent       *) { return PROPAGATE; }
	virtual DispatchResult on_idle        (DispatcherCtx, const IdleEvent        *) { return PROPAGATE; }
	virtual DispatchResult on_quit_request(DispatcherCtx, const QuitRequestEvent *) { return PROPAGATE; }
	virtual DispatchResult on_layout      (DispatcherCtx, const LayoutEvent      *) { return PROPAGATE; }

	void refresh_layout() {
		LayoutEvent e;
		broadcast(resolve_context(NULL), &e);
	}

	virtual DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reverse=false) {
		(void)reverse;
		ctx.clip(get_viewport());
		return e->deliver(ctx, this);
	}

	DispatcherCtx resolve_context(Window *) const;
};

inline DispatchResult MouseMoveEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_mouse_move(ctx, this);
}

inline DispatchResult MouseDownEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_mouse_down(ctx, this);
}

inline DispatchResult MouseUpEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_mouse_up(ctx, this);
}

inline DispatchResult KeyDownEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_key_down(ctx, this);
}

inline DispatchResult KeyUpEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_key_up(ctx, this);
}

inline DispatchResult IdleEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_idle(ctx, this);
}

inline DispatchResult QuitRequestEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_quit_request(ctx, this);
}

inline DispatchResult RenderEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_render(ctx, this);
}

inline DispatchResult LayoutEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_layout(ctx, this);
}
