#pragma once
#include <swuix/widget.hpp>
#include <swuix/state.hpp>

class ScrollableWidget : public virtual Widget {
public:
    Texture *viewport;
    Vec2f    viewport_pos;

    ScrollableWidget(Rect2f frame, Vec2f content_box, Widget *p, State *s)
            : Widget(Rect2f(frame.pos, content_box), p, s) {
        viewport_pos = frame.pos;
        viewport = state->window->CreateTexture();
        viewport->SetSize(frame.size);
    }

    void blit(Texture *target, Vec2f acc) override {
        if (texture_dirty) {
            draw();

            for (int i = children.size() - 1; i >= 0; --i) {
                Widget *child = children[i];
                if (child->isClipped())
                    child->blit(texture, {0, 0});
            }
            texture_dirty = false;
        }

        viewport->Draw(*texture, contentOffset());
        target->Draw(*viewport, acc + viewport_pos);

        for (int i = children.size() - 1; i >= 0; --i) {
            Widget *child = children[i];
            if (!child->isClipped())
                child->blit(target, acc + position);
        }
    }

    Vec2f contentOffset() const {
        return {-contentProgressX(), -contentProgressY()};
    }

    float contentProgressY() const {
        return viewport_pos.y - position.y;
    }

    float contentProgressX() const {
        return viewport_pos.x - position.x;
    }

    void scrollY(float dy) {
        Rect2f f = frame();
        Vec2f viewport_size = viewport->GetSize();
        float new_y = clamp(
            f.pos.y + dy,
            viewport_pos.y + viewport_size.y - f.size.y,
            viewport_pos.y
        );
        position.y = new_y;
        requestLayout();
        requestRedraw();
    }

    void scrollX(float dx) {
        Rect2f f = frame();
        Vec2f viewport_size = viewport->GetSize();
        float new_x = clamp(
            f.pos.x + dx,
            viewport_pos.x + viewport_size.x - f.size.x,
            viewport_pos.x
        );
        position.x = new_x;
        requestLayout();
        requestRedraw();
    }

    DispatchResult onMouseWheel(DispatcherCtx, const MouseWheelEvent *) override;
    DispatchResult onMouseMove (DispatcherCtx, const MouseMoveEvent  *) override;
};
