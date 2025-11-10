#pragma once
#include <swuix/traits/scrollable.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/widgets/titled.hpp>

class TallView : public TitledWidget, public ScrollableWidget {
    VScrollbar *scrollbar;

public:
    TallView(Rect2f frame, Vec2f content_box, Widget *p, State *s)
            : Widget(Rect2f(frame.pos, content_box), p, s),
            TitledWidget(Rect2f(frame.pos, content_box), p, s),
            ScrollableWidget(frame, content_box, p, s) {
        scrollbar = new VScrollbar(state);
        scrollbar->attachTo(this);
    }

    const char *title() const override {
        return "Widget window";
    }

    void blit(Texture *target, Vec2f acc) override {
        if (!minimized) return ScrollableWidget::blit(target, acc);
        titlebar->blit(target, acc);
    }

    void layout() override {
        float progress_px = contentProgressY();
        titlebar->position.y = progress_px - HANDLE_H;
        scrollbar->position.y = progress_px;
        scrollbar->texture->SetSize({scrollbar->texture->GetWidth(), viewport->GetHeight()});
        scrollbar->slider->texture->SetSize({scrollbar->slider->texture->GetWidth(), scrollbar->scrollHeight() * (viewport->GetHeight() / texture->GetHeight())});
        scrollbar->slider->position.y = SCROLL_BUT_H + scrollbar->scrollProgress();
        scrollbar->layout();
    }
};
