#pragma once
#include <swuix/widget.hpp>
#include <swuix/widgets/container.hpp>

class TraitCastError : public std::runtime_error {
public:
    explicit TraitCastError(const std::string &message) : std::runtime_error(message) {}
};

class ControlledWidget;  // forward-declare

class Control : public virtual Widget {
public:
    Control(Rect2F f, Widget *par, State *st) : Widget(f, par, st) {}

    virtual void attach_to(ControlledWidget *host) = 0;
};

class ControlledWidget : public virtual Widget {
protected:
    std::vector<Control*> controls;

public:
    ControlledWidget(Rect2F f, Widget *par, State *st) : Widget(f, par, st) {}

    const char *title() const {
        return "Controlled widget";
    }

    void attach(Control *control) {
        controls.push_back(control);
        control->parent = this;
    }

    DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
        DispatcherCtx local_ctx;

        if (reversed) {
            Rect2F prev_rect = ctx.viewport;
            ctx.clip(get_viewport());

            if (e->deliver(ctx, this) == CONSUME) return CONSUME;

            ctx.setViewport(prev_rect);
            local_ctx = ctx.withOffset(frame);

            for (int i = (int)controls.size() - 1; i >= 0; --i) {
                if (controls[i]->broadcast(local_ctx, e, true) == CONSUME) {
                    return CONSUME;
                }
            }

            return PROPAGATE;
        }

        local_ctx = ctx.withOffset(frame);

        for (size_t i = 0; i < controls.size(); ++i) {
            if (controls[i]->broadcast(local_ctx, e) == CONSUME) {
                return CONSUME;
            }
        }

        ctx.clip(get_viewport());

        return e->deliver(ctx, this);
    }
};

class ControlledContainer : public ControlledWidget, public WidgetContainer {
public:
    ControlledContainer(Rect2F f, Widget *par, State *st)
        : Widget(f, par, st), ControlledWidget(f, par, st), WidgetContainer(f, par, st) {}
    ControlledContainer(Rect2F f, Widget *par, std::vector<Widget*> children_, State *st)
        : Widget(f, par, st), ControlledWidget(f, par, st), WidgetContainer(f, par, children_, st) {}

    const char *title() const {
        return "Controlled container";
    }

    DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
        DispatcherCtx local_ctx;

        if (reversed) {
            Rect2F prev_rect = ctx.viewport;
            ctx.clip(get_viewport());

            if (e->deliver(ctx, this) == CONSUME) return CONSUME;

            local_ctx = ctx.withOffset(frame);

            for (int i = (int)children.size() - 1; i >= 0; --i) {
                if (children[i]->broadcast(local_ctx, e, true) == CONSUME) {
                    return CONSUME;
                }
            }

            ctx.setViewport(prev_rect);
            local_ctx = ctx.withOffset(frame);

            for (int i = (int)controls.size() - 1; i >= 0; --i) {
                if (controls[i]->broadcast(local_ctx, e, true) == CONSUME) {
                    return CONSUME;
                }
            }

            return PROPAGATE;
        }

        local_ctx = ctx.withOffset(frame);

        for (size_t i = 0; i < controls.size(); ++i) {
            if (controls[i]->broadcast(local_ctx, e) == CONSUME) {
                return CONSUME;
            }
        }

        ctx.clip(get_viewport());
        local_ctx = ctx.withOffset(frame);

        for (size_t i = 0; i < children.size(); ++i) {
            if (children[i]->broadcast(local_ctx, e) == CONSUME) {
                return CONSUME;
            }
        }

        return e->deliver(ctx, this);
    }
};
