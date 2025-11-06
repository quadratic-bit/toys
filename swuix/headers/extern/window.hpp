#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <optional>
#include <stdexcept>
#include <string>

#include "dr4/window.hpp"
#include "dr4/texture.hpp"
#include "dr4/math/color.hpp"
#include "dr4/math/vec2.hpp"

#include "texture.hpp"

static inline uint32_t utf8_first_codepoint(const char* s) {
    if (!s || !*s) return 0;
    const unsigned char c = static_cast<unsigned char>(s[0]);
    if ((c & 0x80u) == 0x00u) {
        return c;
    } else if ((c & 0xE0u) == 0xC0u) {
        unsigned char c1 = static_cast<unsigned char>(s[1]);
        if ((c1 & 0xC0u) != 0x80u) return 0;
        return ((c & 0x1Fu) << 6) | (c1 & 0x3Fu);
    } else if ((c & 0xF0u) == 0xE0u) {
        unsigned char c1 = static_cast<unsigned char>(s[1]);
        unsigned char c2 = static_cast<unsigned char>(s[2]);
        if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u) return 0;
        return ((c & 0x0Fu) << 12) | ((c1 & 0x3Fu) << 6) | (c2 & 0x3Fu);
    } else if ((c & 0xF8u) == 0xF0u) {
        unsigned char c1 = static_cast<unsigned char>(s[1]);
        unsigned char c2 = static_cast<unsigned char>(s[2]);
        unsigned char c3 = static_cast<unsigned char>(s[3]);
        if ((c1 & 0xC0u) != 0x80u || (c2 & 0xC0u) != 0x80u || (c3 & 0xC0u) != 0x80u) return 0;
        return ((c & 0x07u) << 18) | ((c1 & 0x3Fu) << 12) | ((c2 & 0x3Fu) << 6) | (c3 & 0x3Fu);
    }
    return 0;
}

class SwuixWindow : public dr4::Window {
public:
    explicit SwuixWindow(dr4::Vec2f size,
                         std::string title = "swuix",
                         const char *font_path = "/usr/share/fonts/TTF/CaskaydiaCoveNerdFontMono-Regular.ttf",
                         int font_px = 16)
        : window_(NULL),
          renderer_(NULL),
          text_engine_(NULL),
          font_(NULL),
          size_(size),
          title_(title),
          font_path_(font_path),
          font_px_(font_px),
          is_open_(false) {}

    ~SwuixWindow() {
        Close();
    }

    // ------------- dr4::Window -------------

    void SetTitle(const std::string &title) {
        title_ = title;
        if (window_) SDL_SetWindowTitle(window_, title_.c_str());
    }

    const std::string &GetTitle() const {
        return title_;
    }

    dr4::Vec2f GetSize() const {
        if (!window_) return size_;
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        return dr4::Vec2f(static_cast<float>(w), static_cast<float>(h));
    }

    void SetSize(const ::dr4::Vec2f& size) {
        size_ = size;
        if (window_) {
            SDL_SetWindowSize(window_, static_cast<int>(SDL_ceilf(size.x)),
                                       static_cast<int>(SDL_ceilf(size.y)));
        }
    }

    void Open() {
        if (is_open_) return;

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(SDL_GetError());
        }

        if (!SDL_CreateWindowAndRenderer(
                title_.c_str(),
                static_cast<int>(size_.x),
                static_cast<int>(size_.y),
                SDL_WINDOW_RESIZABLE,
                &window_, &renderer_
        )) {
            std::string err = SDL_GetError();
            SDL_Quit();
            throw std::runtime_error(err);
        }

        if (!TTF_Init()) {
            std::string err = SDL_GetError();
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            window_ = NULL;
            renderer_ = NULL;
            SDL_Quit();
            throw std::runtime_error(err);
        }
        text_engine_ = TTF_CreateRendererTextEngine(renderer_);
        if (!text_engine_) {
            std::string err = SDL_GetError();
            TTF_Quit();
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            window_ = NULL;
            renderer_ = NULL;
            SDL_Quit();
            throw std::runtime_error(err);
        }
        font_ = TTF_OpenFont(font_path_, font_px_);
        if (!font_) {
            std::string err = SDL_GetError();
            TTF_DestroyRendererTextEngine(text_engine_);
            text_engine_ = NULL;
            TTF_Quit();
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            window_ = NULL;
            renderer_ = NULL;
            SDL_Quit();
            throw std::runtime_error(err);
        }

        is_open_ = true;
    }

    bool IsOpen() const {
        return is_open_;
    }

    void Close() {
        if (!is_open_) return;

        if (font_) {
            TTF_CloseFont(font_);
            font_ = NULL;
        }
        if (text_engine_) {
            TTF_DestroyRendererTextEngine(text_engine_);
            text_engine_ = NULL;
        }
        TTF_Quit();

        if (renderer_) {
            SDL_DestroyRenderer(renderer_);
            renderer_ = NULL;
        }
        if (window_) {
            SDL_DestroyWindow(window_);
            window_ = NULL;
        }
        SDL_Quit();

        is_open_ = false;
    }

    void Clear(const dr4::Color &color) {
        ensureOpen_();
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(
            renderer_,
            toU8(color.r),
            toU8(color.g),
            toU8(color.b),
            toU8(color.a)
        );
        SDL_RenderClear(renderer_);
    }

    void Draw(const dr4::Texture &texture, dr4::Vec2f pos) {
        ensureOpen_();

        const SwuixTexture *sw = static_cast<const SwuixTexture*>(&texture);
        if (!sw) return;

        SDL_Texture *sdltex = sw->GetSDLTexture();
        if (!sdltex) return;

        float tw = 0, th = 0;
        SDL_GetTextureSize(sdltex, &tw, &th);
        SDL_FRect dst{ pos.x, pos.y, tw, th };
        SDL_SetTextureBlendMode(sdltex, SDL_BLENDMODE_BLEND);
        SDL_RenderTexture(renderer_, sdltex, NULL, &dst);
    }

    void Display() {
        ensureOpen_();
        SDL_RenderPresent(renderer_);
    }

    dr4::Texture *CreateTexture() {
        return new SwuixTexture(renderer_, text_engine_, font_);
    }

    dr4::Image *CreateImage() {
        return new SwuixImage();
    }

    dr4::Font *CreateFont() {
        return new SwuixFont();
    }

    std::optional<dr4::Event> PollEvent() {
        ensureOpen_();

        SDL_Event e;
        if (!SDL_PollEvent(&e)) return std::nullopt;

        dr4::Event out;

        switch (e.type) {
        case SDL_EVENT_QUIT:
            out.type = dr4::Event::Type::QUIT;
            return out;

        case SDL_EVENT_WINDOW_RESIZED: {
            size_ = dr4::Vec2f(
                static_cast<float>(e.window.data1),
                static_cast<float>(e.window.data2)
            );
        } break;

        case SDL_EVENT_MOUSE_MOTION: {
            out.type = dr4::Event::Type::MOUSE_MOVE;
            out.mouseMove.pos = dr4::Vec2f(
                static_cast<float>(e.motion.x),
                static_cast<float>(e.motion.y)
            );
            out.mouseMove.rel = dr4::Vec2f(
                static_cast<float>(e.motion.xrel),
                static_cast<float>(e.motion.yrel)
            );
            return out;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            out.type = (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                       ? dr4::Event::Type::MOUSE_DOWN
                       : dr4::Event::Type::MOUSE_UP;
            out.mouseButton.button = static_cast<dr4::MouseCode>(e.button.button);
            out.mouseButton.pos = dr4::Vec2f(
                static_cast<float>(e.button.x),
                static_cast<float>(e.button.y)
            );
            return out;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
            out.type = dr4::Event::Type::MOUSE_WHEEL;
            float mx = 0.f, my = 0.f;
            SDL_GetMouseState(&mx, &my);
            out.mouseWheel.delta = static_cast<int>(e.wheel.y);
            out.mouseWheel.pos   = dr4::Vec2f(mx, my);
            return out;
        }
        case SDL_EVENT_TEXT_INPUT: {
            out.type = dr4::Event::Type::TEXT_EVENT;
            const char *txt = e.text.text;
            out.text.unicode = utf8_first_codepoint(txt);
            return out;
        }

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            out.type = (e.type == SDL_EVENT_KEY_DOWN)
                     ? dr4::Event::Type::KEY_DOWN
                     : dr4::Event::Type::KEY_UP;
            out.key.sym = static_cast<dr4::KeyCode>(e.key.key);
            out.key.mod = static_cast<dr4::KeyMode>(SDL_GetModState());
            return out;
        }

        default:
            break;
        }

        return std::nullopt;
    }

    SDL_Renderer   *GetRenderer()   const { return renderer_; }
    SDL_Window     *GetSDLWindow()  const { return window_; }
    TTF_TextEngine *GetTextEngine() const { return text_engine_; }
    TTF_Font       *GetFont()       const { return font_; }

private:
    SDL_Window     *window_;
    SDL_Renderer   *renderer_;
    TTF_TextEngine *text_engine_;
    TTF_Font       *font_;

    dr4::Vec2f   size_;
    std::string  title_;
    const char  *font_path_;
    int          font_px_;
    bool         is_open_;

    static Uint8 toU8(int v) {
        if (v < 0) return 0;
        if (v > 255) return 255;
        return static_cast<Uint8>(v);
    }

    void ensureOpen_() const {
        if (!is_open_ || !window_ || !renderer_) {
            throw std::runtime_error("SwuixWindow: window not open");
        }
    }
};
