#pragma once
#include <vector>

#include <swuix/common.hpp>
#include <swuix/geometry.hpp>
#include <dr4/texture.hpp>

using dr4::Vec2f;
using dr4::Rect2f;
using dr4::Color;
using dr4::Text;
using dr4::Texture;
using dr4::Rectangle;

using std::vector;

struct State;

struct DispatcherCtx {
    Vec2f    mouse_rel;
    Rect2f   surface;
    State   *state;

    void clip(Rect2f rect) {
        surface = intersect(surface, rect);
    }

    DispatcherCtx withOffset(Vec2f pos) const {
        DispatcherCtx c = *this;
        c.mouse_rel.x -= pos.x;
        c.mouse_rel.y -= pos.y;
        c.surface.pos.x -= pos.x;
        c.surface.pos.y -= pos.y;
        return c;
    }

    static DispatcherCtx fromAbsolute(Vec2f abs, Rect2f clip, State *s) {
        DispatcherCtx c;
        c.mouse_rel = abs;
        c.surface = clip;
        c.state = s;
        return c;
    }
};

enum DispatchResult {
    PROPAGATE,
    CONSUME
};

class Widget;

struct Event {
    virtual ~Event() {}
    virtual DispatchResult deliver(DispatcherCtx ctx, Widget *w) = 0;
};

struct IdleEvent : public Event {
    Time dt_s;
    Time budget_s;
    Time deadline;
    IdleEvent(Time dt, Time budget, Time deadline_)
        : dt_s(dt), budget_s(budget), deadline(deadline_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct QuitRequestEvent : Event {
    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseMoveEvent : Event {
    Vec2f mouse_abs;
    MouseMoveEvent(Vec2f mouse_abs_) : mouse_abs(mouse_abs_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseDownEvent : Event {
    Vec2f mouse_abs;
    MouseDownEvent(Vec2f mouse_abs_) : mouse_abs(mouse_abs_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseUpEvent : Event {
    MouseUpEvent() {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct MouseWheelEvent : Event {
    Vec2f delta;
    MouseWheelEvent(Vec2f delta_) : delta(delta_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct InputEvent : Event {
    const char *text;  // The input text, UTF-8 encoded

    InputEvent(const char *text_) : text(text_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct KeyEvent : Event {
    uint32_t keycode;

    uint16_t mods;  // bitmask

    KeyEvent(int key, uint16_t mod)
        : keycode(key), mods(mod) {}
};

struct KeyDownEvent : KeyEvent {
    bool repeat;
    KeyDownEvent(int key, uint16_t mod, bool repeat_)
        : KeyEvent(key, mod), repeat(repeat_) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

struct KeyUpEvent : KeyEvent {
    KeyUpEvent(int key, uint16_t mod)
        : KeyEvent(key, mod) {}

    DispatchResult deliver(DispatcherCtx ctx, Widget *w);
};

class Widget {
protected:
    bool texture_dirty = true;

public:
    Widget         *parent;
    vector<Widget*> children = vector<Widget*>();
    Vec2f           position;
    Texture        *texture;
    State          *state;

    Widget(Rect2f frame, Widget *p, State *s);
    virtual ~Widget();

    virtual const char *title() const = 0;

    // ============== Children =============

    void appendChild(Widget *child) {
        children.push_back(child);
        child->parent = this;
    }
    
    void clearChildren() {
        for (Widget *child : children) {
            delete child;
        }
        children.clear();
    }

    // ============ Positioning ============

    virtual bool isClipped() const { return true; }

    // What area should clip input at this level
    virtual Rect2f inputClip() const { return frame(); }

    virtual void translate(Vec2f new_pos) {
        position = new_pos;
        parent->requestLayout();
    }

    virtual void resize(Vec2f new_size) {
        texture->SetSize(new_size);
        requestLayout();
    }

    void setFrame(Rect2f new_frame) {
        translate(new_frame.pos);
        resize(new_frame.size);
    }
    Rect2f frame() const {
        return {position, texture->GetSize()};
    }

    bool containsMouse(DispatcherCtx ctx) const {
        return ctx.surface.Contains(ctx.mouse_rel) && frame().Contains(ctx.mouse_rel);
    }

    // ============= Rendering =============

    virtual void draw() = 0;

    virtual void blit(Texture *target, Vec2f acc = {0, 0}) {
        if (texture_dirty) {
            draw();

            for (int i = children.size() - 1; i >= 0; --i) {
                Widget *child = children[i];
                if (child->isClipped())
                    child->blit(texture, {0, 0});
            }
            texture_dirty = false;
        }

        dr4::Vec2f old_pos = tempPos(texture, acc + position);
        if (target != texture) target->Draw(*texture);
        texture->SetPos(old_pos);

        for (int i = children.size() - 1; i >= 0; --i) {
            Widget *child = children[i];
            if (!child->isClipped())
                child->blit(target, acc + position);
        }
    }

    DispatcherCtx resolveContext() const;

    void requestRedraw() {
        texture_dirty = true;
        if (parent != this) parent->requestRedraw();
    }

    // =============== Layout ==============

    virtual void layout() {}

    void requestLayout() {
        layout();
        for (Widget *child : children) {
            child->layout();
        }
    }

    // =============== Events ==============

    virtual DispatchResult broadcast(DispatcherCtx ctx, Event *e) {
        for (Widget *child : children) {
            if (child->isClipped()) ctx.clip(frame());
            DispatcherCtx local_ctx = ctx.withOffset(position);
            if (child->broadcast(local_ctx, e) == CONSUME) return CONSUME;
        }

        return e->deliver(ctx, this);
    }

    virtual DispatchResult onMouseMove  (DispatcherCtx, const MouseMoveEvent   *);
    virtual DispatchResult onMouseDown  (DispatcherCtx, const MouseDownEvent   *) { return PROPAGATE; }
    virtual DispatchResult onMouseUp    (DispatcherCtx, const MouseUpEvent     *) { return PROPAGATE; }
    virtual DispatchResult onMouseWheel (DispatcherCtx, const MouseWheelEvent  *) { return PROPAGATE; }
    virtual DispatchResult onInput      (DispatcherCtx, const InputEvent       *) { return PROPAGATE; }
    virtual DispatchResult onKeyDown    (DispatcherCtx, const KeyDownEvent     *) { return PROPAGATE; }
    virtual DispatchResult onKeyUp      (DispatcherCtx, const KeyUpEvent       *) { return PROPAGATE; }
    virtual DispatchResult onIdle       (DispatcherCtx, const IdleEvent        *) { return PROPAGATE; }
    virtual DispatchResult onQuitRequest(DispatcherCtx, const QuitRequestEvent *) { return PROPAGATE; }
};

inline DispatchResult MouseMoveEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onMouseMove(ctx, this);
}

inline DispatchResult MouseDownEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onMouseDown(ctx, this);
}

inline DispatchResult MouseUpEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onMouseUp(ctx, this);
}

inline DispatchResult MouseWheelEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onMouseWheel(ctx, this);
}

inline DispatchResult InputEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onInput(ctx, this);
}

inline DispatchResult KeyDownEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onKeyDown(ctx, this);
}

inline DispatchResult KeyUpEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onKeyUp(ctx, this);
}

inline DispatchResult IdleEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onIdle(ctx, this);
}

inline DispatchResult QuitRequestEvent::deliver(DispatcherCtx ctx, Widget *w) {
    return w->onQuitRequest(ctx, this);
}
