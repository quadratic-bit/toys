#pragma once
#include <swuix/widgets/titlebar.hpp>

class TitledWidget : public MinimizableWidget {
protected:
    TitleBar *titlebar;

public:
    TitledWidget(Rect2f f, Widget *p, State *s) : Widget(f, p, s), MinimizableWidget(f, p, s) {
        titlebar = new TitleBar(state);
        titlebar->attachTo(this);
    }

    virtual void blit(Texture *target, Vec2f acc) override {
        if (!minimized) return Widget::blit(target, acc);
        titlebar->blit(target, acc + position);
    }

    virtual DispatchResult broadcast(DispatcherCtx ctx, Event *e) override {
        if (!minimized) return Widget::broadcast(ctx, e);
        return titlebar->broadcast(ctx.withOffset(position), e);
    }

    void layout() override {
        titlebar->texture->SetSize({texture->GetWidth(), HANDLE_H});
        titlebar->position.x = 0;
        titlebar->position.y = -HANDLE_H;
    }
};
