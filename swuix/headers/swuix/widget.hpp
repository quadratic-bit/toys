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
    Vec2F  mouseRel;
    Vec2F  mouseAbs;
    Rect2F viewport;
    Window *window;

    Rect2F prev_clip;

    ~DispatcherCtx() {
        if (window) window->clip(transform(prev_clip));
    }

    void setViewport(Rect2F frame) {
        viewport = frame;
        if (window) window->clip(transform(viewport));
    }

    void clip(Rect2F frame) {
        viewport = intersect(viewport, frame);
        if (window) window->clip(transform(viewport));
    }

    Vec2F offset() const {
        return mouseAbs - mouseRel;
    }

    Rect2F transform(Rect2F rect) const {
        Vec2F off = offset();
        rect.x += off.x;
        rect.y += off.y;
        return rect;
    }

    DispatcherCtx withOffset(Rect2F frame) const {
        DispatcherCtx c = *this;
        c.mouseRel.x -= frame.x;
        c.mouseRel.y -= frame.y;
        c.viewport.x -= frame.x;
        c.viewport.y -= frame.y;
        c.prev_clip.x -= frame.x;
        c.prev_clip.y -= frame.y;
        return c;
    }

    static DispatcherCtx fromAbsolute(Vec2F abs, Rect2F clip, Window *window) {
        DispatcherCtx c;
        c.mouseAbs = c.mouseRel = abs;
        c.viewport = c.prev_clip = clip;
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
    Vec2F mouse_abs;
    MouseMoveEvent(Vec2F mouse_abs_) : mouse_abs(mouse_abs_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseDownEvent : Event {
    Vec2F mouse_abs;
    MouseDownEvent(Vec2F mouse_abs_) : mouse_abs(mouse_abs_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseUpEvent : Event {
    MouseUpEvent() {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseWheelEvent : Event {
    Vec2F delta;
    MouseWheelEvent(Vec2F delta_) : delta(delta_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct KeyEvent : Event {
    unsigned scancode;   // platform-neutral
    uint32_t keycode;

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
    Rect2F  frame;  // `x` and `y` are relative to `parent`

    Widget(Rect2F frame_, Widget *parent_, State *state_)
        : parent(parent_ ? parent_ : this), state(state_), frame(frame_) {};

    virtual ~Widget();

    template<typename T, size_t N>
    static std::vector<T> makeChildren(T const (&arr)[N]) {
        return std::vector<T>(arr, arr + N);
    }

    virtual Rect2F getViewport() const {
        Rect2F v = frame;
        v.x -= 1;
        v.y -= 1;
        v.w += 2;
        v.h += 2;
        return v;
    }
    virtual void set_frame(Rect2F new_frame) { frame = new_frame; }

    virtual const char *title() const = 0;

    virtual bool contains_mouse(DispatcherCtx ctx) const {
        Vec2F rel = ctx.mouseRel;
        Rect2F view = ctx.viewport;
        return rel.x >= view.x && rel.x <= view.x + view.w
            && rel.y >= view.y && rel.y <= view.y + view.h;
    }

    virtual void render(Window *window, float off_x, float off_y) = 0;

    virtual DispatchResult on_render(DispatcherCtx ctx, const RenderEvent *) {
        Vec2F off = ctx.offset();
        render(ctx.window, off.x, off.y);

        return PROPAGATE;
    }

    virtual DispatchResult on_mouse_move  (DispatcherCtx, const MouseMoveEvent   *);
    virtual DispatchResult on_mouse_down  (DispatcherCtx, const MouseDownEvent   *) { return PROPAGATE; }
    virtual DispatchResult on_mouse_up    (DispatcherCtx, const MouseUpEvent     *) { return PROPAGATE; }
    virtual DispatchResult on_mouse_wheel (DispatcherCtx, const MouseWheelEvent  *) { return PROPAGATE; }
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
        ctx.clip(getViewport());
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

inline DispatchResult MouseWheelEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->on_mouse_wheel(ctx, this);
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
