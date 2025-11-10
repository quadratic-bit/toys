#pragma once

#include "dr4/window.hpp"
#include "dr4/math/rect.hpp"

typedef double Time;  // monotonic time, presented in seconds

static const dr4::Color TRANSPARENT = {0, 0, 0, 0};

inline dr4::Vec2f tempPos(dr4::Texture *tex, dr4::Vec2f temp_pos) {
    dr4::Vec2f old_pos = tex->GetPos();
    tex->SetPos(temp_pos);
    return old_pos;
}

inline dr4::Rectangle *rectFill(dr4::Window *w, dr4::Rect2f frame, dr4::Color fill) {
    dr4::Rectangle *r = w->CreateRectangle();
    r->SetPos({0, 0});
    r->SetSize(frame.size);
    r->SetFillColor(fill);
    r->SetBorderColor(TRANSPARENT);
    return r;
}

inline dr4::Rectangle *rectBorder(dr4::Window *w, dr4::Rect2f frame, dr4::Color fill, float thick, dr4::Color border) {
    dr4::Rectangle *r = w->CreateRectangle();
    r->SetPos({thick / 2, thick / 2});
    r->SetSize({frame.size.x - thick, frame.size.y - thick});
    r->SetFillColor(fill);
    r->SetBorderThickness(thick);
    r->SetBorderColor(border);
    return r;
}

inline dr4::Rectangle *outline(dr4::Window *w, dr4::Rect2f frame, float thick, dr4::Color border) {
    dr4::Rectangle *r = w->CreateRectangle();
    r->SetPos({thick / 2, thick / 2});
    r->SetSize({frame.size.x - thick, frame.size.y - thick});
    r->SetFillColor(TRANSPARENT);
    r->SetBorderThickness(thick);
    r->SetBorderColor(border);
    return r;
}

enum class HAlign { LEFT, CENTER, RIGHT };
inline dr4::Text *textAligned(
        dr4::Window *w,
        const char *s,
        dr4::Vec2f pos,
        dr4::Color color,
        const dr4::Font *font,
        float font_size = 16,
        HAlign halign = HAlign::LEFT,
        dr4::Text::VAlign valign = dr4::Text::VAlign::MIDDLE
) {
    dr4::Text *t = w->CreateText();
    t->SetText(s);
    t->SetColor(color);
    t->SetFontSize(font_size);
    t->SetFont(font);
    t->SetVAlign(valign);

    dr4::Vec2f bounds = t->GetBounds();
    float x = pos.x;
    if (halign == HAlign::CENTER)      x -= bounds.x * 0.5f;
    else if (halign == HAlign::RIGHT)  x -= bounds.x;

    t->SetPos({x, pos.y});
    return t;
}
