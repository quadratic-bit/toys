#pragma once
#include <SDL3/SDL_rect.h>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include "geometry.hpp"

// forward-declare
class Window;
class Widget;
class State;

struct DispatcherCtx {
	Vec2 local;
	Vec2 absolute;

	DispatcherCtx with_offset(float dx, float dy) const {
		DispatcherCtx c = *this;
		c.local.x -= dx;
		c.local.y -= dy;
		return c;
	}

	static DispatcherCtx from_absolute(float ax, float ay) {
		DispatcherCtx c;
		c.absolute.x = ax;
		c.absolute.y = ay;
		c.local.x = ax;
		c.local.y = ay;
		return c;
	}
};

enum DispatchResult {
	PROPAGATE,
	CONSUME
};

struct Event {
	virtual ~Event() {}
	virtual bool is_pointer() const { return false; }
	virtual DispatchResult deliver(DispatcherCtx ctx, Widget *w) = 0;
};

struct MouseMoveEvent : public Event {
	float ax, ay;  // absolute
	MouseMoveEvent(float x, float y) : ax(x), ay(y) {}
	bool is_pointer() const { return true; }
	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseDownEvent : public Event {
	float ax, ay;  // absolute
	MouseDownEvent(float x, float y) : ax(x), ay(y) {}
	bool is_pointer() const { return true; }
	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseUpEvent : public Event {
	MouseUpEvent() {}
	DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

class Widget {
public:
	Widget *parent;
	State  *state;
	SDL_FRect frame;  // `x` and `y` are relative to `parent`

	Widget(SDL_FRect frame_, Widget *parent_, State *state_) : parent(parent_), state(state_), frame(frame_) {};
	virtual ~Widget() {}

	template<typename T, size_t N>
	static std::vector<T> make_children(T const (&arr)[N]) {
		return std::vector<T>(arr, arr + N);
	}

	virtual size_t child_count() const { return 0; }
	virtual Widget *child_at(size_t) const { return 0; }
	virtual bool is_leaf() const { return child_count() == 0; }

	bool contains_point(Vec2 rel) {
		return rel.x >= frame.x && rel.x <= frame.x + frame.w
			&& rel.y >= frame.y && rel.y <= frame.y + frame.h;
	}

	virtual void render(Window *window, int off_x, int off_y) = 0;

	virtual DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e);
	virtual DispatchResult on_mouse_down(DispatcherCtx ctx, const MouseDownEvent *e) { (void)e; (void)ctx; return PROPAGATE; }
	virtual DispatchResult on_mouse_up  (DispatcherCtx ctx, const MouseUpEvent   *e) { (void)e; (void)ctx; return PROPAGATE; }

	virtual DispatchResult route(DispatcherCtx ctx, Event *e) {
		DispatcherCtx here = ctx.with_offset(frame.x, frame.y);

		const size_t n = child_count();

		// TODO: generalize
		if (e->is_pointer()) {
			for (size_t i = 0; i < n; ++i) {
				Widget *ch = child_at(i);
				DispatchResult r = ch->route(here, e);
				if (r == CONSUME) {
					return CONSUME;
				}
			}
		} else {
			for (size_t i = 0; i < n; ++i) {
				if (child_at(i)->route(here, e) == CONSUME) {
					return CONSUME;
				}
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
