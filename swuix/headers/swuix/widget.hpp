#pragma once
#include <cstdio>
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

	DispatcherCtx with_offset(Point2f dp) const {
		DispatcherCtx c = *this;
		c.mouse_rel -= dp;
		return c;
	}

	static DispatcherCtx from_absolute(Point2f abs) {
		DispatcherCtx c;
		c.mouse_abs = c.mouse_rel = abs;
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

struct IdleEvent : public Event {
	float dt_s;
	IdleEvent(float dt_s_) : dt_s(dt_s_) {}
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
	int scancode;  // platform-neutral
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
		: parent(parent_), state(state_), frame(frame_) {};
	virtual ~Widget() {
		size_t n = child_count();
		for (size_t i = 0; i < n; ++i) {
			delete child_at(i);
		}
	}

	template<typename T, size_t N>
	static std::vector<T> make_children(T const (&arr)[N]) {
		return std::vector<T>(arr, arr + N);
	}

	virtual size_t child_count() const { return 0; }
	virtual Widget *child_at(size_t) const { return 0; }
	virtual bool is_leaf() const { return child_count() == 0; }

	virtual const char *title() const = 0;

	bool contains_point(Point2f rel) {
		return rel.x >= frame.x && rel.x <= frame.x + frame.w
			&& rel.y >= frame.y && rel.y <= frame.y + frame.h;
	}

	virtual void render(Window *window, int off_x, int off_y) = 0;

	virtual DispatchResult on_mouse_move  (DispatcherCtx, const MouseMoveEvent   *);
	virtual DispatchResult on_mouse_down  (DispatcherCtx, const MouseDownEvent   *) { return PROPAGATE; }
	virtual DispatchResult on_mouse_up    (DispatcherCtx, const MouseUpEvent     *) { return PROPAGATE; }
	virtual DispatchResult on_key_down    (DispatcherCtx, const KeyDownEvent     *) { return PROPAGATE; }
	virtual DispatchResult on_key_up      (DispatcherCtx, const KeyUpEvent       *) { return PROPAGATE; }
	virtual DispatchResult on_idle        (DispatcherCtx, const IdleEvent        *) { return PROPAGATE; }
	virtual DispatchResult on_quit_request(DispatcherCtx, const QuitRequestEvent *) { return PROPAGATE; }

	virtual DispatchResult route(DispatcherCtx ctx, Event *e) {
		DispatcherCtx here = ctx.with_offset(Point2f(frame.x, frame.y));

		const size_t n = child_count();

		for (size_t i = 0; i < n; ++i) {
			if (child_at(i)->route(here, e) == CONSUME) {
				return CONSUME;
			}
		}

		return e->deliver(ctx, this);
	}
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
	DispatchResult res = w->on_idle(ctx, this);
	//printf("[ON_IDLE] (%s) %s\n", typeid(*w).name(), w->title());
	return res;
}

inline DispatchResult QuitRequestEvent::deliver(DispatcherCtx ctx, Widget *w) {
	return w->on_quit_request(ctx, this);
}
