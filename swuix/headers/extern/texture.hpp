#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>

#include "dr4/texture.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/math/color.hpp"

inline SDL_FRect frect(float x, float y, float w, float h) {
    SDL_FRect r;
    r.x = x; r.y = y; r.w = w; r.h = h;
    return r;
}

class SwuixFont final : public dr4::Font {
    TTF_Font *font_;

    static void ensure_(bool ok, const char *what) {
        if (!ok) throw std::runtime_error(std::string(what) + ": " + SDL_GetError());
    }

public:
    SwuixFont() : font_(nullptr) {}
    ~SwuixFont() override {
        if (font_) TTF_CloseFont(font_);
    }

    void LoadFromFile(const std::string &path) override {
        if (font_) {
            TTF_CloseFont(font_);
            font_ = nullptr;
        }
        font_ = TTF_OpenFont(path.c_str(), 10);
        ensure_(font_ != nullptr, "SwuixFont: TTF_OpenFont failed");
    }

    void LoadFromBuffer(const void *buffer, size_t size) override {
        if (font_) {
            TTF_CloseFont(font_);
            font_ = nullptr;
        }
        SDL_IOStream *io = SDL_IOFromConstMem(buffer, static_cast<int>(size));
        if (!io) throw std::runtime_error(std::string("SDL_IOFromConstMem failed: ") + SDL_GetError());
        font_ = TTF_OpenFontIO(io, /*closeio*/ true, /*ptsize*/ 10);
        if (!font_) throw std::runtime_error(std::string("TTF_OpenFontIO failed: ") + SDL_GetError());
    }

    float GetAscent(float fontSize) const override {
        if (!font_) return 0.0f;

        const int prev = TTF_GetFontSize(font_);
        if (prev != static_cast<int>(fontSize))
            TTF_SetFontSize(font_, static_cast<int>(fontSize));

        const int ascent = TTF_GetFontAscent(font_);
        if (prev != static_cast<int>(fontSize))
            TTF_SetFontSize(font_, prev);

        return static_cast<float>(ascent);
    }

    float GetDescent(float fontSize) const override {
        if (!font_) return 0.0f;

        const int prev = TTF_GetFontSize(font_);
        if (prev != static_cast<int>(fontSize))
            TTF_SetFontSize(font_, static_cast<int>(fontSize));

        const int descent = TTF_GetFontDescent(font_);
        if (prev != static_cast<int>(fontSize))
            TTF_SetFontSize(font_, prev);

        return static_cast<float>(descent);
    }

    TTF_Font *raw() const {
        return font_;
    }
};

class SwuixImage final : public dr4::Image {
    std::vector<uint8_t> pixels_;  // RGBA, 8bpc
    int w_, h_;
    dr4::Vec2f pos_;

public:
    SwuixImage() : w_(0), h_(0), pos_(0.0f, 0.0f) {}
    ~SwuixImage() override = default;

    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }
    dr4::Vec2f GetPos() const override { return pos_; }

    void SetPixel(size_t x, size_t y, dr4::Color color) override {
        if (x >= static_cast<size_t>(w_) || y >= static_cast<size_t>(h_)) return;
        const size_t idx = (y * static_cast<size_t>(w_) + x) * 4u;
        pixels_[idx + 0] = color.r;
        pixels_[idx + 1] = color.g;
        pixels_[idx + 2] = color.b;
        pixels_[idx + 3] = color.a;
    }

    dr4::Color GetPixel(size_t x, size_t y) const override {
        if (x >= static_cast<size_t>(w_) || y >= static_cast<size_t>(h_)) {
            return dr4::Color(0, 0, 0, 0);
        }
        const size_t idx = (y * static_cast<size_t>(w_) + x) * 4u;
        return dr4::Color(pixels_[idx + 0], pixels_[idx + 1], pixels_[idx + 2], pixels_[idx + 3]);
    }

    void SetSize(dr4::Vec2f size) override {
        const int nw = std::max(0, static_cast<int>(SDL_ceilf(size.x)));
        const int nh = std::max(0, static_cast<int>(SDL_ceilf(size.y)));
        if (nw == 0 || nh == 0) {
            w_ = h_ = 0;
            pixels_.clear();
            return;
        }
        w_ = nw; h_ = nh;
        pixels_.assign(static_cast<size_t>(w_) * static_cast<size_t>(h_) * 4u, 0u);
    }

    dr4::Vec2f GetSize() const override {
        return dr4::Vec2f(static_cast<float>(w_), static_cast<float>(h_));
    }
    float GetWidth()  const override { return static_cast<float>(w_); }
    float GetHeight() const override { return static_cast<float>(h_); }

    const void *Pixels() const { return pixels_.empty() ? nullptr : pixels_.data(); }
    int Pitch() const { return w_ * 4; }
    int Width() const { return w_; }
    int Height() const { return h_; }

    void DrawOn(dr4::Texture &texture) const override;
};

class SwuixLine final : public dr4::Line {
    dr4::Vec2f pos_  {0,0};
    dr4::Vec2f start_{0,0};
    dr4::Vec2f end_  {0,0};
    dr4::Color color_{0,0,0,255};
    float thickness_ {1.0f};

public:
    ~SwuixLine() override = default;

    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }
    dr4::Vec2f GetPos() const override { return pos_; }

    void SetStart(dr4::Vec2f start) override { start_ = start; }
    void SetEnd(dr4::Vec2f end) override { end_ = end; }
    void SetColor(dr4::Color color) override { color_ = color; }
    void SetThickness(float t) override { thickness_ = t; }

    dr4::Vec2f GetStart() const override { return start_; }
    dr4::Vec2f GetEnd() const override { return end_; }
    dr4::Color GetColor() const override { return color_; }
    float GetThickness() const override { return thickness_; }

    void DrawOn(dr4::Texture &texture) const override;
};

class SwuixCircle final : public dr4::Circle {
    dr4::Vec2f center_{0,0};
    float radius_     {0.0f};
    dr4::Color fill_  {0,0,0,255};
    dr4::Color border_{0,0,0,255};
    float border_t_   {0.0f};

public:
    ~SwuixCircle() override = default;

    void SetPos(dr4::Vec2f pos) override { center_ = {pos.x + radius_, pos.y + radius_}; }
    dr4::Vec2f GetPos() const override { return {center_.x - radius_, center_.y - radius_}; }

    void SetCenter(dr4::Vec2f c) override { center_ = c; }
    void SetRadius(float r) override { radius_ = r; }
    void SetFillColor(dr4::Color c) override { fill_ = c; }
    void SetBorderColor(dr4::Color c) override { border_ = c; }
    void SetBorderThickness(float t) override { border_t_ = t; }

    dr4::Vec2f GetCenter() const override { return center_; }
    float GetRadius() const override { return radius_; }
    dr4::Color GetFillColor() const override { return fill_; }
    dr4::Color GetBorderColor() const override { return border_; }
    float GetBorderThickness() const override { return border_t_; }

    void DrawOn(dr4::Texture &texture) const override;
};

class SwuixRectangle final : public dr4::Rectangle {
    dr4::Vec2f pos_   {0,0};
    dr4::Vec2f size_  {0,0};
    dr4::Color fill_  {0,0,0,255};
    dr4::Color border_{0,0,0,0};
    float border_t_   {0.0f};

public:
    ~SwuixRectangle() override = default;

    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }
    dr4::Vec2f GetPos() const override { return pos_; }

    void SetSize(dr4::Vec2f size) override { size_ = size; }
    void SetFillColor(dr4::Color color) override { fill_ = color; }
    void SetBorderThickness(float t) override { border_t_ = t; }
    void SetBorderColor(dr4::Color color) override { border_ = color; }

    dr4::Vec2f GetSize() const override { return size_; }
    dr4::Color GetFillColor() const override { return fill_; }
    float GetBorderThickness() const override { return border_t_; }
    dr4::Color GetBorderColor() const override { return border_; }

    void DrawOn(dr4::Texture &texture) const override;
};

class SwuixText final : public dr4::Text {
    dr4::Vec2f pos_  {0,0};
    std::string text_;
    dr4::Color color_{255,255,255,255};
    float font_size_ {16.0f};
    VAlign valign_   {VAlign::BASELINE};
    const dr4::Font *font_{nullptr}; // non-owning

public:
    ~SwuixText() override = default;

    void SetPos(dr4::Vec2f pos) override { pos_ = pos; }
    dr4::Vec2f GetPos() const override { return pos_; }

    void SetText(const std::string &text) override { text_ = text; }
    void SetColor(dr4::Color color) override { color_ = color; }
    void SetFontSize(float size) override { font_size_ = size; }
    void SetVAlign(VAlign a) override { valign_ = a; }
    void SetFont(const dr4::Font *f) override { font_ = f; }

    dr4::Vec2f GetBounds() const override {
        const SwuixFont *sf = (font_) ? dynamic_cast<const SwuixFont*>(font_) : nullptr;
        if (!sf || !sf->raw() || text_.empty()) return dr4::Vec2f(0, 0);

        TTF_Font *raw = sf->raw();
        const int prev = TTF_GetFontSize(raw);
        if (prev != static_cast<int>(font_size_)) TTF_SetFontSize(raw, static_cast<int>(font_size_));

        int w = 0, h = 0;
        if (!TTF_GetStringSize(raw, text_.c_str(), text_.length(), &w, &h)) {
            w = 0;
            h = 0;
        }
        if (prev != static_cast<int>(font_size_)) TTF_SetFontSize(raw, prev);
        return dr4::Vec2f(static_cast<float>(w), static_cast<float>(h));
    }

    const std::string &GetText() const override { return text_; }
    dr4::Color GetColor() const override { return color_; }
    float GetFontSize() const override { return font_size_; }
    VAlign GetVAlign() const override { return valign_; }
    const dr4::Font &GetFont() const override {
        if (!font_) throw std::runtime_error("SwuixText: font not set");
        return *font_;
    }

    void DrawOn(dr4::Texture &texture) const override;
};

class SwuixTexture final : public dr4::Texture {
    SDL_Renderer  *renderer_;
    SDL_Texture   *tex_;
    dr4::Vec2f     size_;
    dr4::Vec2f     pos_;
    dr4::Vec2f     zero_;
    TTF_TextEngine *text_engine_;

    static Uint8 toU8(unsigned int v) {
        return static_cast<Uint8>(v > 255u ? 255u : v);
    }

    void ensureTarget_() const {
        if (!tex_) throw std::runtime_error("SwuixTexture not initialized: call SetSize() first");
    }

    void destroyTexture_() {
        if (tex_) {
            SDL_DestroyTexture(tex_);
            tex_ = nullptr;
        }
    }

public:
    SwuixTexture(SDL_Renderer *renderer, TTF_TextEngine *text_engine = nullptr)
    : renderer_(renderer), tex_(nullptr), size_(0.f, 0.f), pos_(0.f, 0.f), zero_(0.f, 0.f), text_engine_(text_engine) {
        if (!renderer_) throw std::runtime_error("SwuixTexture: renderer is null");
    }
    ~SwuixTexture() override { destroyTexture_(); }

    void SetPos(dr4::Vec2f p) override { pos_ = p; }
    dr4::Vec2f GetPos() const override { return pos_; }

    void SetSize(dr4::Vec2f size) override {
        int w = static_cast<int>(SDL_ceilf(size.x));
        int h = static_cast<int>(SDL_ceilf(size.y));
        if (w <= 0) w = 1;
        if (h <= 0) h = 1;

        if (tex_) {
            float tw = 0, th = 0;
            SDL_GetTextureSize(tex_, &tw, &th);
            if (tw == w && th == h) {
                size_ = size;
                return;
            }
            destroyTexture_();
        }

        tex_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, w, h);
        if (!tex_)
            throw std::runtime_error(std::string("SDL_CreateTexture failed: ") + SDL_GetError());

        SDL_SetTextureBlendMode(tex_, SDL_BLENDMODE_BLEND);
        size_ = size;

        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
        SDL_RenderClear(renderer_);
        SDL_SetRenderTarget(renderer_, prev);
    }

    dr4::Vec2f GetSize()   const override { return size_; }
    float      GetWidth()  const override { return size_.x; }
    float      GetHeight() const override { return size_.y; }

    void       SetZero(dr4::Vec2f pos)       override { zero_ = pos; }
    dr4::Vec2f GetZero()               const override { return zero_; }

    void Clear(dr4::Color color) override {
        ensureTarget_();
        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer_);
        SDL_SetRenderTarget(renderer_, prev);
    }

    void DrawOn(dr4::Texture &texture) const override {
        const SwuixTexture *src = this;
        SwuixTexture *dst = dynamic_cast<SwuixTexture*>(&texture);
        if (!dst || !src->GetSDLTexture()) return;

        SDL_Renderer *ren    = dst->GetSDLRenderer();
        SDL_Texture  *dstTex = dst->GetSDLTexture();
        SDL_Texture  *srcTex = src->GetSDLTexture();

        float sw = 0.f, sh = 0.f;
        SDL_GetTextureSize(srcTex, &sw, &sh);

        SDL_FRect srcRect{
            src->pos_.x + dst->zero_.x,
            src->pos_.y + dst->zero_.y,
            sw, sh
        };

        SDL_Texture *prev = SDL_GetRenderTarget(ren);
        SDL_SetRenderTarget(ren, dstTex);
        SDL_SetTextureBlendMode(srcTex, SDL_BLENDMODE_BLEND);
        SDL_RenderTexture(ren, srcTex, nullptr, &srcRect);
        SDL_SetRenderTarget(ren, prev);
    }

    SDL_Renderer   *GetSDLRenderer() const { return renderer_; }
    SDL_Texture    *GetSDLTexture()  const { return tex_; }
    TTF_TextEngine *GetTextEngine()  const { return text_engine_; }
};

inline void SwuixImage::DrawOn(dr4::Texture &texture) const {
    SwuixTexture *dst = dynamic_cast<SwuixTexture*>(&texture);
    if (!dst || Width() <= 0 || Height() <= 0 || !Pixels()) return;

    SDL_Surface *surf = SDL_CreateSurfaceFrom(
        Width(), Height(), SDL_PIXELFORMAT_RGBA32,
        const_cast<void*>(Pixels()), Pitch());
    if (!surf) return;

    SDL_Texture *tmp = SDL_CreateTextureFromSurface(dst->GetSDLRenderer(), surf);
    SDL_DestroySurface(surf);
    if (!tmp) return;

    SDL_FRect dst_rect = frect(this->GetPos().x + dst->GetZero().x,
                               this->GetPos().y + dst->GetZero().y,
                               static_cast<float>(Width()),
                               static_cast<float>(Height()));

    SDL_Texture *prev = SDL_GetRenderTarget(dst->GetSDLRenderer());
    SDL_SetRenderTarget(dst->GetSDLRenderer(), dst->GetSDLTexture());
    SDL_SetTextureBlendMode(tmp, SDL_BLENDMODE_BLEND);
    SDL_RenderTexture(dst->GetSDLRenderer(), tmp, nullptr, &dst_rect);
    SDL_SetRenderTarget(dst->GetSDLRenderer(), prev);
    SDL_DestroyTexture(tmp);
}

inline void SwuixRectangle::DrawOn(dr4::Texture &texture) const {
    SwuixTexture *dst = dynamic_cast<SwuixTexture*>(&texture);
    if (!dst) return;

    SDL_Texture *prev = SDL_GetRenderTarget(dst->GetSDLRenderer());
    SDL_SetRenderTarget(dst->GetSDLRenderer(), dst->GetSDLTexture());
    SDL_SetRenderDrawBlendMode(dst->GetSDLRenderer(), SDL_BLENDMODE_BLEND);

    const float x = pos_.x + dst->GetZero().x;
    const float y = pos_.y + dst->GetZero().y;
    const float w = size_.x;
    const float h = size_.y;

    if (fill_.a != 0 && w > 0 && h > 0) {
        SDL_SetRenderDrawColor(dst->GetSDLRenderer(), fill_.r, fill_.g, fill_.b, fill_.a);
        SDL_FRect r = frect(x, y, w, h);
        SDL_RenderFillRect(dst->GetSDLRenderer(), &r);
    }

    if (border_t_ > 0.0f && border_.a != 0 && w > 0 && h > 0) {
        SDL_SetRenderDrawColor(dst->GetSDLRenderer(), border_.r, border_.g, border_.b, border_.a);
        const float t    = border_t_;
        SDL_FRect top    = frect(x,         y,         w, t);
        SDL_FRect bottom = frect(x,         y + h - t, w, t);
        SDL_FRect left   = frect(x,         y,         t, h);
        SDL_FRect right  = frect(x + w - t, y,         t, h);
        if (top.w    > 0 && top.h    > 0) SDL_RenderFillRect(dst->GetSDLRenderer(), &top);
        if (bottom.w > 0 && bottom.h > 0) SDL_RenderFillRect(dst->GetSDLRenderer(), &bottom);
        if (left.w   > 0 && left.h   > 0) SDL_RenderFillRect(dst->GetSDLRenderer(), &left);
        if (right.w  > 0 && right.h  > 0) SDL_RenderFillRect(dst->GetSDLRenderer(), &right);
    }

    SDL_SetRenderTarget(dst->GetSDLRenderer(), prev);
}

inline void SwuixText::DrawOn(dr4::Texture &texture) const {
    SwuixTexture *dst = dynamic_cast<SwuixTexture*>(&texture);
    if (!dst || text_.empty() || !font_) return;

    const SwuixFont *sf = dynamic_cast<const SwuixFont*>(font_);
    if (!sf || !sf->raw()) return;

    TTF_Font *use = sf->raw();
    const int prev_px = TTF_GetFontSize(use);
    if (prev_px != static_cast<int>(font_size_)) TTF_SetFontSize(use, static_cast<int>(font_size_));

    TTF_Text *tt = TTF_CreateText(dst->GetTextEngine(), use, text_.c_str(), static_cast<int>(text_.size()));
    if (!tt) {
        if (prev_px != static_cast<int>(font_size_)) TTF_SetFontSize(use, prev_px);
        return;
    }

    TTF_SetTextColor(tt, color_.r, color_.g, color_.b, color_.a);

    int tw = 0, th = 0;
    TTF_GetTextSize(tt, &tw, &th);

    float draw_x = pos_.x + dst->GetZero().x;
    float draw_y = pos_.y + dst->GetZero().y;

    switch (valign_) {
        case VAlign::TOP:      break;
        case VAlign::MIDDLE:   draw_y -= th * 0.5f; break;
        case VAlign::BASELINE: draw_y -= static_cast<float>(TTF_GetFontAscent(use)); break;
        case VAlign::BOTTOM:   draw_y -= static_cast<float>(th); break;
        default:               break;
    }

    SDL_Texture *prev = SDL_GetRenderTarget(dst->GetSDLRenderer());
    SDL_SetRenderTarget(dst->GetSDLRenderer(), dst->GetSDLTexture());
    TTF_DrawRendererText(tt, draw_x, draw_y);
    SDL_SetRenderTarget(dst->GetSDLRenderer(), prev);

    TTF_DestroyText(tt);
    if (prev_px != static_cast<int>(font_size_)) TTF_SetFontSize(use, prev_px);
}

inline void SwuixLine::DrawOn(dr4::Texture &texture) const {
    SwuixTexture *dst = dynamic_cast<SwuixTexture*>(&texture);
    if (!dst) return;

    const float x1 = pos_.x + start_.x + dst->GetZero().x;
    const float y1 = pos_.y + start_.y + dst->GetZero().y;
    const float x2 = pos_.x + end_.x   + dst->GetZero().x;
    const float y2 = pos_.y + end_.y   + dst->GetZero().y;

    SDL_Texture *prev = SDL_GetRenderTarget(dst->GetSDLRenderer());
    SDL_SetRenderTarget(dst->GetSDLRenderer(), dst->GetSDLTexture());

    if (thickness_ > 1.0f) {
        thickLineRGBA(dst->GetSDLRenderer(), x1, y1, x2, y2, thickness_,
                      color_.r, color_.g, color_.b, color_.a);
    } else {
        lineRGBA(dst->GetSDLRenderer(), x1, y1, x2, y2,
                 color_.r, color_.g, color_.b, color_.a);
    }

    SDL_SetRenderTarget(dst->GetSDLRenderer(), prev);
}

inline void SwuixCircle::DrawOn(dr4::Texture &texture) const {
    SwuixTexture *dst = dynamic_cast<SwuixTexture*>(&texture);
    if (!dst || radius_ <= 0.0f) return;

    const float cx = center_.x + dst->GetZero().x;
    const float cy = center_.y + dst->GetZero().y;
    const int   r  = std::max(0, static_cast<int>(std::floor(radius_)));

    SDL_Texture *prev = SDL_GetRenderTarget(dst->GetSDLRenderer());
    SDL_SetRenderTarget(dst->GetSDLRenderer(), dst->GetSDLTexture());

    // Fill
    if (fill_.a != 0) {
        filledCircleRGBA(dst->GetSDLRenderer(),
                         cx, cy, r,
                         fill_.r, fill_.g, fill_.b, fill_.a);
    }

    // Border thickness (approximate by concentric rings)
    if (border_t_ > 0.0f && border_.a != 0) {
        const int t = std::max(1, static_cast<int>(std::floor(border_t_)));
        for (int i = 0; i < t; ++i) {
            const int rr = std::max(0, r - i);
            circleRGBA(dst->GetSDLRenderer(),
                       cx, cy, rr,
                       border_.r, border_.g, border_.b, border_.a);
        }
    }

    SDL_SetRenderTarget(dst->GetSDLRenderer(), prev);
}
