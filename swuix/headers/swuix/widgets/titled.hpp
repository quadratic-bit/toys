#pragma once
#include <swuix/widgets/titlebar.hpp>

class TitledWidget : public MinimizableWidget, public ControlledWidget {
protected:
    TitleBar *titlebar;

public:
    TitledWidget(Rect2F content_frame_, Widget *parent_, State *state_)
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

        DispatcherCtx local_ctx = ctx.withOffset(frame);

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
};

class TitledContainer : public MinimizableWidget, public ControlledContainer {
protected:
    TitleBar *titlebar;

public:
    TitledContainer(Rect2F content_frame_, Widget *parent_, State *state_)
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

        DispatcherCtx local_ctx = ctx.withOffset(frame);

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
