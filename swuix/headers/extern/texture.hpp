#pragma once
#include <stdexcept>

#include "dr4/texture.hpp"
#include "SDL3_ttf/SDL_ttf.h"
#include "swuix/geometry.hpp"

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

    void Draw(const dr4::Rectangle &rect) {
        ensureTarget_();

        SDL_Texture *prev = SDL_GetRenderTarget(renderer_);
        SDL_SetRenderTarget(renderer_, tex_);
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

        if (rect.fill.a != 0) {
            SDL_SetRenderDrawColor(
                renderer_,
                toU8(rect.fill.r),
                toU8(rect.fill.g),
                toU8(rect.fill.b),
                toU8(rect.fill.a)
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
                toU8(rect.borderColor.r),
                toU8(rect.borderColor.g),
                toU8(rect.borderColor.b),
                toU8(rect.borderColor.a)
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

        const int prev_px = TTF_GetFontSize(base_font_);
        if (prev_px != static_cast<int>(text.fontSize)) {
            TTF_SetFontSize(base_font_, static_cast<int>(text.fontSize));
        }

        TTF_Text *tt = TTF_CreateText(text_engine_, base_font_, text.text.c_str(), static_cast<int>(text.text.size()));
        if (!tt) {
            if (prev_px != static_cast<int>(text.fontSize)) TTF_SetFontSize(base_font_, prev_px);
            return;
        }

        TTF_SetTextColor(tt, toU8(text.color.r), toU8(text.color.g), toU8(text.color.b), toU8(text.color.a));

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
            int ascent = TTF_GetFontAscent(base_font_);
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
        if (prev_px != static_cast<int>(text.fontSize)) TTF_SetFontSize(base_font_, prev_px);
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
    static Uint8 toU8(int v) {
        if (v < 0) return 0;
        if (v > 255) return 255;
        return static_cast<Uint8>(v);
    }

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
