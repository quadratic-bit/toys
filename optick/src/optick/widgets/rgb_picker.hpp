#pragma once
#include <algorithm>

#include <swuix/widget.hpp>
#include <swuix/traits/focusable.hpp>
#include <swuix/state.hpp>
#include <swuix/common.hpp>
#include <swuix/window/common.hpp>

class RGBPicker;

class RGBSlider final : public FocusableWidget {
public:
    enum class Channel { R, G, B };

private:
    RGBPicker *picker_{nullptr};
    Channel channel_;
    bool dragging_ = false;

public:
    RGBSlider(Rect2f f, Widget *p, State *s, RGBPicker *picker, Channel ch)
        : Widget(f, p, s)
        , FocusableWidget(f, p, s)
        , picker_(picker)
        , channel_(ch)
    {}

    const char *title() const override { return "RGBSlider"; }

    void draw() override;

    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *e) override;
    DispatchResult onMouseUp  (DispatcherCtx ctx, const MouseUpEvent   *e) override;
    DispatchResult onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) override;

private:
    uint8_t channelValue() const;
    void setFromMouse(DispatcherCtx ctx);

    // draw a cheap gradient with a fixed number of steps
    void drawGradientBar(Rect2f bar, dr4::Color left, dr4::Color right);
    void drawKnob(Rect2f bar);
};

class RGBPicker : public Widget {
    friend class RGBSlider;

    uint8_t r_{255}, g_{0}, b_{0};

    RGBSlider *sR_{nullptr};
    RGBSlider *sG_{nullptr};
    RGBSlider *sB_{nullptr};

protected:
    virtual void onColorChanged(dr4::Color) {}

public:
    RGBPicker(Rect2f f, Widget *p, State *s)
        : Widget(f, p, s)
    {
        // initial child frames will be corrected in layout()
        sR_ = new RGBSlider({0, 0, f.size.x, 24}, this, state, this, RGBSlider::Channel::R);
        sG_ = new RGBSlider({0, 0, f.size.x, 24}, this, state, this, RGBSlider::Channel::G);
        sB_ = new RGBSlider({0, 0, f.size.x, 24}, this, state, this, RGBSlider::Channel::B);

        appendChild(sR_);
        appendChild(sG_);
        appendChild(sB_);

        requestLayout();
    }

    const char *title() const override { return "RGBPicker"; }

    dr4::Color getColor() const { return {r_, g_, b_, 255}; }

    void setColor(dr4::Color c) {
        r_ = c.r; g_ = c.g; b_ = c.b;
        requestRedraw();
        for (auto *ch : children) ch->requestRedraw();
        onColorChanged(getColor());
    }

    void layout() override {
        Rect2f f = frame();
        const float pad = 8.0f;
        const float headerH = 14.0f;
        const float rowH = 24.0f;
        const float gap  = 4.0f;

        const float previewW = 44.0f;

        float contentW = std::max(60.0f, f.size.x - previewW - pad * 3.0f);
        float x = pad;
        float y = pad + headerH + gap;

        sR_->position.x = x;
        sR_->position.y = y;
        sR_->texture->SetSize(contentW, rowH);
        y += rowH + gap;

        sG_->position.x = x;
        sG_->position.y = y;
        sG_->texture->SetSize(contentW, rowH);
        y += rowH + gap;

        sB_->position.x = x;
        sB_->position.y = y;
        sB_->texture->SetSize(contentW, rowH);

        requestRedraw();
    }

    void draw() override {
        Rect2f f { 0, 0, texture->GetWidth(), texture->GetHeight() };

        texture->Clear({CLR_SURFACE_2});
        Rectangle *rect = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*rect);

        const float pad = 6.0f;
        const float previewW = 44.0f;

        Rect2f swatch {
            f.size.x - previewW - pad,
            pad,
            previewW,
            f.size.y - pad * 2.0f
        };

        rect->SetPos(swatch.pos);
        rect->SetSize(swatch.size);
        rect->SetFillColor({r_, g_, b_});
        texture->Draw(*rect);

        rect->SetFillColor(TRANSPARENT);
        rect->SetBorderColor({CLR_BORDER});
        rect->SetBorderThickness(1);
        texture->Draw(*rect);

        Text *t = textAligned(
            state->window,
            "RGB",
            {pad + 2.0f, pad + 10.0f},
            Color(CLR_TEXT_STRONG),
            state->appfont
        );
        texture->Draw(*t);
    }

private:
    uint8_t getChannel(RGBSlider::Channel ch) const {
        switch (ch) {
            case RGBSlider::Channel::R: return r_;
            case RGBSlider::Channel::G: return g_;
            case RGBSlider::Channel::B: return b_;
        }
        return 0;
    }

    void setChannel(RGBSlider::Channel ch, uint8_t v) {
        switch (ch) {
            case RGBSlider::Channel::R: r_ = v; break;
            case RGBSlider::Channel::G: g_ = v; break;
            case RGBSlider::Channel::B: b_ = v; break;
        }
        requestRedraw();
        if (sR_) sR_->requestRedraw();
        if (sG_) sG_->requestRedraw();
        if (sB_) sB_->requestRedraw();
        onColorChanged(getColor());
    }
};
