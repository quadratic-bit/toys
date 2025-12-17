#include "./rgb_picker.hpp"

static float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

uint8_t RGBSlider::channelValue() const {
    if (!picker_) return 0;
    return picker_->getChannel(channel_);
}

void RGBSlider::setFromMouse(DispatcherCtx ctx) {
    if (!picker_) return;

    Rect2f f = frame();

    const float pad = 18.0f;
    float x0 = pad;
    float x1 = f.size.x - 8.0f;
    float w  = std::max(1.0f, x1 - x0);

    float local_x = ctx.mouse_rel.x - position.x;
    float mx = clampf(local_x, x0, x1);
    float t  = (mx - x0) / w;

    int val = (int)(t * 255.0f + 0.5f);
    val = std::max(0, std::min(255, val));

    picker_->setChannel(channel_, (uint8_t)val);
}

DispatchResult RGBSlider::onMouseDown(DispatcherCtx ctx, const MouseDownEvent *e) {
    FocusableWidget::onMouseDown(ctx, e);

    if (!containsMouse(ctx)) return PROPAGATE;

    dragging_ = true;
    setFromMouse(ctx);
    return CONSUME;
}

DispatchResult RGBSlider::onMouseUp(DispatcherCtx, const MouseUpEvent *) {
    if (dragging_) {
        dragging_ = false;
        return CONSUME;
    }
    return PROPAGATE;
}

DispatchResult RGBSlider::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) {
    if (dragging_) {
        setFromMouse(ctx);
        Widget::onMouseMove(ctx, e);
        return CONSUME;
    }
    return Widget::onMouseMove(ctx, e);
}

void RGBSlider::drawGradientBar(Rect2f bar, dr4::Color left, dr4::Color right) {
    const int steps = 48;
    if (steps <= 0 || bar.size.x <= 1.0f || bar.size.y <= 1.0f) return;

    const int x0 = (int)bar.pos.x;
    const int x1 = (int)(bar.pos.x + bar.size.x);
    const int w  = std::max(1, x1 - x0);

    Rectangle *rr;
    rr = rectFill(state->window, {0, 0, 0, 0}, {0, 0, 0});
    for (int i = 0; i < steps; ++i) {
        // integer tiling to avoid float rounding artifacts
        int sx = x0 + (i * w) / steps;
        int ex = x0 + ((i + 1) * w) / steps;
        int sw = std::max(1, ex - sx);

        float t = (steps == 1) ? 0.0f : (float)i / (float)(steps - 1);

        int r = (int)(left.r + t * ((int)right.r - (int)left.r) + 0.5f);
        int g = (int)(left.g + t * ((int)right.g - (int)left.g) + 0.5f);
        int b = (int)(left.b + t * ((int)right.b - (int)left.b) + 0.5f);

        r = std::clamp(r, 0, 255);
        g = std::clamp(g, 0, 255);
        b = std::clamp(b, 0, 255);

        Rect2f seg {
            (float)sx,
            bar.pos.y,
            (float)sw,
            bar.size.y
        };

        rr->SetSize(seg.size);
        rr->SetPos(seg.pos);
        rr->SetFillColor({(uint8_t)r, (uint8_t)g, (uint8_t)b});
        texture->Draw(*rr);
    }

    rr->SetSize(bar.size);
    rr->SetPos(bar.pos);
    rr->SetFillColor(TRANSPARENT);
    rr->SetBorderColor({CLR_BORDER});
    rr->SetBorderThickness(1);
    texture->Draw(*rr);
}

void RGBSlider::drawKnob(Rect2f bar) {
    uint8_t v = channelValue();
    float t = (float)v / 255.0f;

    float x = bar.pos.x + t * bar.size.x;

    dr4::Line *l = thickLine(
        state->window,
        {x, bar.pos.y - 2.0f},
        {x, bar.pos.y + bar.size.y + 2.0f},
        {CLR_BORDER},
        2
    );
    texture->Draw(*l);
}

void RGBSlider::draw() {
    Rect2f f { 0, 0, texture->GetWidth(), texture->GetHeight() };

    texture->Clear({CLR_SURFACE_2});

    Rectangle *box = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
    texture->Draw(*box);

    const char *lbl = "R";
    if (channel_ == Channel::G) lbl = "G";
    if (channel_ == Channel::B) lbl = "B";

    Text *t = textAligned(
        state->window,
        lbl,
        {6.0f, 12.0f},
        Color(CLR_TEXT_STRONG),
        state->appfont
    );
    texture->Draw(*t);

    uint8_t r = picker_ ? picker_->r_ : 0;
    uint8_t g = picker_ ? picker_->g_ : 0;
    uint8_t b = picker_ ? picker_->b_ : 0;

    dr4::Color left{r, g, b, 255};
    dr4::Color right{r, g, b, 255};

    switch (channel_) {
        case Channel::R: left.r = 0; right.r = 255; break;
        case Channel::G: left.g = 0; right.g = 255; break;
        case Channel::B: left.b = 0; right.b = 255; break;
    }

    const float barH = 8.0f;
    const float barY = (f.size.y - barH) * 0.5f;

    Rect2f bar {
        18.0f,
        barY,
        f.size.x - 26.0f,
        barH
    };

    drawGradientBar(bar, left, right);
    drawKnob(bar);
}
