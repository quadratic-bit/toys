#pragma once
#include <cassert>

#include <swuix/widget.hpp>

class WidgetContainer : public virtual Widget {
protected:
    std::vector<Widget *> children;

public:
    WidgetContainer(Rect2F f, Widget *par, State *st) : Widget(f, par, st) {}
    WidgetContainer(Rect2F f, Widget *par, std::vector<Widget *> children_, State *st)
        : Widget(f, par, st), children(children_) {}

    ~WidgetContainer() {
        for (int i = 0; i < (int)children.size(); ++i) {
            delete children[i];
        }
    }

    const char *title() const {
        return "Container";
    }

    void append_children(const std::vector<Widget *> &new_children) {
        for (size_t ch = 0; ch < new_children.size(); ++ch) {
            Widget *child = new_children[ch];
            child->parent = this;
            children.push_back(child);
        }
    }

    void append_child(Widget *new_child) {
        children.push_back(new_child);
        new_child->parent = this;
    }

    void prepend_child(Widget *new_child) {
        children.insert(children.begin(), new_child);
        new_child->parent = this;
    }

    void remove_child(Widget *prune) {
        for (size_t ch = 0; ch < children.size(); ++ch) {
            Widget *child = children[ch];
            if (child != prune) continue;
            children.erase(children.begin() + ch, children.begin() + ch + 1);
            return;
        }
        // FIXME: is this okay?
        assert(0);
    }

    DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
        ctx.clip(getViewport());

        DispatcherCtx local_ctx = ctx.withOffset(frame);

        if (reversed) {
            if (e->deliver(ctx, this) == CONSUME) return CONSUME;

            for (int i = (int)children.size() - 1; i >= 0; --i) {
                if (children[i]->broadcast(local_ctx, e, true) == CONSUME) {
                    return CONSUME;
                }
            }

            return PROPAGATE;
        }

        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i]->broadcast(local_ctx, e) == CONSUME) {
                return CONSUME;
            }
        }

        return e->deliver(ctx, this);
    }
};
