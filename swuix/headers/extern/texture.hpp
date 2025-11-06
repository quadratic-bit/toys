#pragma once
#include <stdexcept>
#include <vector>

#include "dr4/texture.hpp"
#include "SDL3_ttf/SDL_ttf.h"
#include "swuix/geometry.hpp"

class SwuixFont : public dr4::Font {
    TTF_Font *font_;

public:
    SwuixFont() : font_(NULL) {}

    ~SwuixFont() {
        if (font_) TTF_CloseFont(font_);
    }

    void loadFromFile(const std::string &path) {
        if (font_) {
            TTF_CloseFont(font_);
            font_ = NULL;
        }
        font_ = TTF_OpenFont(path.c_str(), 16);
        if (!font_) {
            throw std::runtime_error(std::string("SwuixFont: TTF_OpenFont failed: ") + SDL_GetError());
        }
    }

    TTF_Font *raw() const {
        return font_;
    }
};

class SwuixImage : public dr4::Image {
    std::vector<uint8_t> pixels_;  // RGBA, 8bpc
    int w_, h_;

public:
    SwuixImage() : w_(0), h_(0) {}

    void SetPixel(unsigned x, unsigned y, dr4::Color color) {
        if (x >= static_cast<unsigned>(w_) || y >= static_cast<unsigned>(h_)) return;
        const size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(w_) + static_cast<size_t>(x)) * 4u;
        pixels_[idx + 0] = color.r;
        pixels_[idx + 1] = color.g;
        pixels_[idx + 2] = color.b;
        pixels_[idx + 3] = color.a;
    }

    dr4::Color GetPixel(unsigned x, unsigned y) const {
        if (x >= static_cast<unsigned>(w_) || y >= static_cast<unsigned>(h_)) {
            return dr4::Color(0, 0, 0, 0);
        }
        const size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(w_) + static_cast<size_t>(x)) * 4u;
        return dr4::Color(pixels_[idx + 0], pixels_[idx + 1], pixels_[idx + 2], pixels_[idx + 3]);
    }

    void SetSize(dr4::Vec2f size) {
        int nw = static_cast<int>(SDL_ceilf(size.x));
        int nh = static_cast<int>(SDL_ceilf(size.y));
        if (nw <= 0 || nh <= 0) {
            w_ = h_ = 0;
            pixels_.clear();
            return;
        }
        w_ = nw; h_ = nh;
        pixels_.assign(static_cast<size_t>(w_ * h_ * 4), 0);
    }

    dr4::Vec2f GetSize() const {
        return dr4::Vec2f(static_cast<float>(w_), static_cast<float>(h_));
    }

    float GetWidth() const {
        return static_cast<float>(w_);
    }

    float GetHeight() const {
        return static_cast<float>(h_);
    }

    const void *Pixels() const {
        return pixels_.empty() ? NULL : pixels_.data();
    }

    int Pitch() const {
        return w_ * 4;
    }

    int Width() const { return w_; }
    int Height() const { return h_; }
};

class SwuixTexture : public dr4::Texture {
    SDL_Renderer *renderer_;
    SDL_Texture  *tex_;
    dr4::Vec2f    size_;

    TTF_TextEngine *text_engine_;
    TTF_Font       *base_font_;

public:
    SwuixTexture(
            SDL_Renderer *renderer,
            TTF_TextEngine *text_engine = NULL,
            TTF_Font *font = NULL
    ) : renderer_(renderer), tex_(NULL), size_(0.0f, 0.0f), text_engine_(text_engine), base_font_(font) {
        if (!renderer_) throw std::runtime_error("SwuixTexture: renderer is null");
    }

    ~SwuixTexture() {
        destroyTexture_();
    }

    void SetSize(dr4::Vec2f size) {
        const int w = static_cast<int>(SDL_ceilf(size.x));
        const int h = static_cast<int>(SDL_ceilf(size.y));
        if (w <= 0 || h <= 0) {
            destroyTexture_();
            size_ = dr4::Vec2f(0, 0);
            return;
        }

        if (tex_) {
            float tw = 0, th = 0;
            SDL_GetTextureSize(tex_, &tw, &th);
            if (tw == w && th == h) {
                size_ = size;
                return;
            }
            destroyTexture_();
        }

        tex_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            w, h
        );
        if (!tex_) {
            throw std::runtime_error(std::string("SDL_CreateTexture failed: ") + SDL_GetError());
        }

        SDL_SetTextureBlendMode(tex_, SDL_BLENDMODE_BLEND);

        size_ = size;

        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
        SDL_RenderClear(renderer_);
        SDL_SetRenderTarget(renderer_, prev);
    }

    dr4::Vec2f GetSize()   const { return size_; }
    float      GetWidth()  const { return size_.x; }
    float      GetHeight() const { return size_.y; }

    void Clear(dr4::Color color) {
        ensureTarget_();
        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer_);
        SDL_SetRenderTarget(renderer_, prev);
    }

    void Draw(const dr4::Rectangle &rect) {
        ensureTarget_();

        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

        if (rect.fill.a != 0) {
            SDL_SetRenderDrawColor(
                renderer_,
                rect.fill.r,
                rect.fill.g,
                rect.fill.b,
                rect.fill.a
            );
            SDL_FRect r = frect(rect.rect.pos.x, rect.rect.pos.y, rect.rect.size.x, rect.rect.size.y);
            if (r.w > 0 && r.h > 0) SDL_RenderFillRect(renderer_, &r);
        }

        if (rect.borderThickness > 0.0f && rect.borderColor.a != 0) {
            const float t = rect.borderThickness;
            const float x = rect.rect.pos.x;
            const float y = rect.rect.pos.y;
            const float w = rect.rect.size.x;
            const float h = rect.rect.size.y;

            SDL_SetRenderDrawColor(
                renderer_,
                rect.borderColor.r,
                rect.borderColor.g,
                rect.borderColor.b,
                rect.borderColor.a
            );

            SDL_FRect top    = frect(x, y, w, t);
            SDL_FRect bottom = frect(x, y + h - t, w, t);
            SDL_FRect left   = frect(x, y, t, h);
            SDL_FRect right  = frect(x + w - t, y, t, h);

            if (top.w    > 0 && top.h    > 0) SDL_RenderFillRect(renderer_, &top);
            if (bottom.w > 0 && bottom.h > 0) SDL_RenderFillRect(renderer_, &bottom);
            if (left.w   > 0 && left.h   > 0) SDL_RenderFillRect(renderer_, &left);
            if (right.w  > 0 && right.h  > 0) SDL_RenderFillRect(renderer_, &right);
        }

        SDL_SetRenderTarget(renderer_, prev);
    }

    void Draw(const dr4::Text &text) {
        ensureTarget_();
        if (!text_engine_ || !base_font_) return;

        TTF_Font *font_to_use = base_font_;
        if (text.font) {
            if (auto sw = static_cast<const SwuixFont*>(text.font)) {
                if (sw->raw()) font_to_use = sw->raw();
            }
        }
        if (!font_to_use) return;

        const int prev_px = TTF_GetFontSize(font_to_use);
        if (prev_px != static_cast<int>(text.fontSize)) {
            TTF_SetFontSize(font_to_use, static_cast<int>(text.fontSize));
        }

        TTF_Text *tt = TTF_CreateText(text_engine_, font_to_use, text.text.c_str(), static_cast<int>(text.text.size()));
        if (!tt) {
            if (prev_px != static_cast<int>(text.fontSize)) TTF_SetFontSize(font_to_use, prev_px);
            return;
        }

        TTF_SetTextColor(tt, text.color.r, text.color.g, text.color.b, text.color.a);

        int tw = 0, th = 0;
        TTF_GetTextSize(tt, &tw, &th);
        float draw_x = text.pos.x;
        float draw_y = text.pos.y;

        switch (text.valign) {

        case dr4::Text::VAlign::TOP:
            break;

        case dr4::Text::VAlign::MIDDLE:
            draw_y -= th * 0.5f;
            break;

        case dr4::Text::VAlign::BASELINE: {
            int ascent = TTF_GetFontAscent(font_to_use);
            draw_y -= static_cast<float>(ascent);
        } break;

        case dr4::Text::VAlign::BOTTOM:
            draw_y -= static_cast<float>(th);
            break;

        default:
            break;
        }

        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        TTF_DrawRendererText(tt, draw_x, draw_y);
        SDL_SetRenderTarget(renderer_, prev);

        TTF_DestroyText(tt);
        if (prev_px != static_cast<int>(text.fontSize)) TTF_SetFontSize(font_to_use, prev_px);
    }

    void Draw(const dr4::Image &img, const dr4::Vec2f &pos) {
        ensureTarget_();

        const SwuixImage *simg = static_cast<const SwuixImage*>(&img);
        if (!simg || simg->Width() <= 0 || simg->Height() <= 0 || !simg->Pixels()) return;

        SDL_Surface *surf = SDL_CreateSurfaceFrom(
            simg->Width(),
            simg->Height(),
            SDL_PIXELFORMAT_RGBA32,
            const_cast<void*>(simg->Pixels()),
            simg->Pitch()
        );
        if (!surf) return;

        SDL_Texture *tmp = SDL_CreateTextureFromSurface(renderer_, surf);
        SDL_DestroySurface(surf);
        if (!tmp) return;

        SDL_FRect dst = frect(pos.x, pos.y, static_cast<float>(simg->Width()), static_cast<float>(simg->Height()));

        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetTextureBlendMode(tmp, SDL_BLENDMODE_BLEND);
        SDL_RenderTexture(renderer_, tmp, nullptr, &dst);
        SDL_SetRenderTarget(renderer_, prev);

        SDL_DestroyTexture(tmp);
    }

    void Draw(const dr4::Texture &texture, const dr4::Vec2f &pos) {
        ensureTarget_();

        const SwuixTexture *other = dynamic_cast<const SwuixTexture*>(&texture);
        if (!other || !other->tex_) return;

        float sw = 0, sh = 0;
        SDL_GetTextureSize(other->tex_, &sw, &sh);

        SDL_FRect dst = frect(pos.x, pos.y, sw, sh);

        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetTextureBlendMode(other->tex_, SDL_BLENDMODE_BLEND);
        SDL_RenderTexture(renderer_, other->tex_, NULL, &dst);
        SDL_SetRenderTarget(renderer_, prev);
    }

    SDL_Texture *GetSDLTexture() const { return tex_; }

private:
    static Uint8 toU8(unsigned int v) {
        if (v > 255u) return 255u;
        return static_cast<Uint8>(v);
    }

    void ensureTarget_() const {
        if (!tex_) throw std::runtime_error("SwuixTexture not initialized: call SetSize() first");
    }

    void destroyTexture_() {
        if (tex_) {
            SDL_DestroyTexture(tex_);
            tex_ = NULL;
        }
    }
};
