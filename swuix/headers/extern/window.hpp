#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <optional>
#include <stdexcept>
#include <string>

#include "dr4/window.hpp"
#include "dr4/event.hpp"
#include "dr4/keycodes.hpp"
#include "dr4/mouse_buttons.hpp"
#include "dr4/math/vec2.hpp"
#include "dr4/math/color.hpp"

#include "texture.hpp"

static inline dr4::MouseButtonType MapSDLMouseButton(Uint8 b) {
    switch (b) {
        case SDL_BUTTON_LEFT:   return dr4::MouseButtonType::LEFT;
        case SDL_BUTTON_MIDDLE: return dr4::MouseButtonType::MIDDLE;
        case SDL_BUTTON_RIGHT:  return dr4::MouseButtonType::RIGHT;
        default:                return dr4::MouseButtonType::UNKNOWN;
    }
}

static inline uint16_t MapSDLModToDR4(Uint16 sdlmods) {
    using dr4::KeyMode;
    uint16_t out = dr4::KEYMOD_NONE;
    if (sdlmods & SDL_KMOD_LSHIFT) out |= dr4::KEYMOD_LSHIFT;
    if (sdlmods & SDL_KMOD_RSHIFT) out |= dr4::KEYMOD_RSHIFT;
    if (sdlmods & SDL_KMOD_LCTRL)  out |= dr4::KEYMOD_LCTRL;
    if (sdlmods & SDL_KMOD_RCTRL)  out |= dr4::KEYMOD_RCTRL;
    if (sdlmods & SDL_KMOD_LALT)   out |= dr4::KEYMOD_LALT;
    if (sdlmods & SDL_KMOD_RALT)   out |= dr4::KEYMOD_RALT;
    if (sdlmods & SDL_KMOD_CAPS)   out |= dr4::KEYMOD_CAPS;
    return out;
}

static inline dr4::KeyCode MapSDLKey(SDL_Keycode key) {
    using namespace dr4;
    switch (key) {
        // Letters
        case SDLK_A: return KEYCODE_A; case SDLK_B: return KEYCODE_B; case SDLK_C: return KEYCODE_C;
        case SDLK_D: return KEYCODE_D; case SDLK_E: return KEYCODE_E; case SDLK_F: return KEYCODE_F;
        case SDLK_G: return KEYCODE_G; case SDLK_H: return KEYCODE_H; case SDLK_I: return KEYCODE_I;
        case SDLK_J: return KEYCODE_J; case SDLK_K: return KEYCODE_K; case SDLK_L: return KEYCODE_L;
        case SDLK_M: return KEYCODE_M; case SDLK_N: return KEYCODE_N; case SDLK_O: return KEYCODE_O;
        case SDLK_P: return KEYCODE_P; case SDLK_Q: return KEYCODE_Q; case SDLK_R: return KEYCODE_R;
        case SDLK_S: return KEYCODE_S; case SDLK_T: return KEYCODE_T; case SDLK_U: return KEYCODE_U;
        case SDLK_V: return KEYCODE_V; case SDLK_W: return KEYCODE_W; case SDLK_X: return KEYCODE_X;
        case SDLK_Y: return KEYCODE_Y; case SDLK_Z: return KEYCODE_Z;

        // Numbers (row)
        case SDLK_0: return KEYCODE_NUM0; case SDLK_1: return KEYCODE_NUM1; case SDLK_2: return KEYCODE_NUM2;
        case SDLK_3: return KEYCODE_NUM3; case SDLK_4: return KEYCODE_NUM4; case SDLK_5: return KEYCODE_NUM5;
        case SDLK_6: return KEYCODE_NUM6; case SDLK_7: return KEYCODE_NUM7; case SDLK_8: return KEYCODE_NUM8;
        case SDLK_9: return KEYCODE_NUM9;

        // Editing / control
        case SDLK_ESCAPE:       return KEYCODE_ESCAPE;
        case SDLK_LCTRL:        return KEYCODE_LCONTROL;
        case SDLK_LSHIFT:       return KEYCODE_LSHIFT;
        case SDLK_LALT:         return KEYCODE_LALT;
        case SDLK_LGUI:         return KEYCODE_LSYSTEM;
        case SDLK_RCTRL:        return KEYCODE_RCONTROL;
        case SDLK_RSHIFT:       return KEYCODE_RSHIFT;
        case SDLK_RALT:         return KEYCODE_RALT;
        case SDLK_RGUI:         return KEYCODE_RSYSTEM;
        case SDLK_MENU:         return KEYCODE_MENU;
        case SDLK_LEFTBRACKET:  return KEYCODE_LBRACKET;
        case SDLK_RIGHTBRACKET: return KEYCODE_RBRACKET;
        case SDLK_SEMICOLON:    return KEYCODE_SEMICOLON;
        case SDLK_COMMA:        return KEYCODE_COMMA;
        case SDLK_PERIOD:       return KEYCODE_PERIOD;
        case SDLK_APOSTROPHE:   return KEYCODE_QUOTE;
        case SDLK_SLASH:        return KEYCODE_SLASH;
        case SDLK_BACKSLASH:    return KEYCODE_BACKSLASH;
        case SDLK_GRAVE:        return KEYCODE_TILDE;
        case SDLK_EQUALS:       return KEYCODE_EQUAL;
        case SDLK_MINUS:        return KEYCODE_HYPHEN;
        case SDLK_SPACE:        return KEYCODE_SPACE;
        case SDLK_RETURN:       return KEYCODE_ENTER;
        case SDLK_BACKSPACE:    return KEYCODE_BACKSPACE;
        case SDLK_TAB:          return KEYCODE_TAB;
        case SDLK_PAGEUP:       return KEYCODE_PAGEUP;
        case SDLK_PAGEDOWN:     return KEYCODE_PAGEDOWN;
        case SDLK_END:          return KEYCODE_END;
        case SDLK_HOME:         return KEYCODE_HOME;
        case SDLK_INSERT:       return KEYCODE_INSERT;
        case SDLK_DELETE:       return KEYCODE_DELETE;

        // Numpad
        case SDLK_KP_0: return KEYCODE_NUMPAD0; case SDLK_KP_1: return KEYCODE_NUMPAD1;
        case SDLK_KP_2: return KEYCODE_NUMPAD2; case SDLK_KP_3: return KEYCODE_NUMPAD3;
        case SDLK_KP_4: return KEYCODE_NUMPAD4; case SDLK_KP_5: return KEYCODE_NUMPAD5;
        case SDLK_KP_6: return KEYCODE_NUMPAD6; case SDLK_KP_7: return KEYCODE_NUMPAD7;
        case SDLK_KP_8: return KEYCODE_NUMPAD8; case SDLK_KP_9: return KEYCODE_NUMPAD9;

        // Arrows
        case SDLK_LEFT:  return KEYCODE_LEFT;
        case SDLK_RIGHT: return KEYCODE_RIGHT;
        case SDLK_UP:    return KEYCODE_UP;
        case SDLK_DOWN:  return KEYCODE_DOWN;

        // Function keys
        case SDLK_F1:  return KEYCODE_F1;  case SDLK_F2:  return KEYCODE_F2;
        case SDLK_F3:  return KEYCODE_F3;  case SDLK_F4:  return KEYCODE_F4;
        case SDLK_F5:  return KEYCODE_F5;  case SDLK_F6:  return KEYCODE_F6;
        case SDLK_F7:  return KEYCODE_F7;  case SDLK_F8:  return KEYCODE_F8;
        case SDLK_F9:  return KEYCODE_F9;  case SDLK_F10: return KEYCODE_F10;
        case SDLK_F11: return KEYCODE_F11; case SDLK_F12: return KEYCODE_F12;
        case SDLK_F13: return KEYCODE_F13; case SDLK_F14: return KEYCODE_F14;
        case SDLK_F15: return KEYCODE_F15;

        case SDLK_PAUSE: return KEYCODE_PAUSE;
        default: return KEYCODE_UNKNOWN;
    }
}

class SwuixWindow final : public dr4::Window {
public:
    explicit SwuixWindow(dr4::Vec2f size,
                         std::string title = "swuix")
        : window_(nullptr),
          renderer_(nullptr),
          text_engine_(nullptr),
          size_(size),
          title_(std::move(title)),
          is_open_(false) {}

    ~SwuixWindow() override { Close(); }

    // ------------- dr4::Window -------------

    void SetTitle(const std::string &title) override {
        title_ = title;
        if (window_) SDL_SetWindowTitle(window_, title_.c_str());
    }

    const std::string &GetTitle() const override { return title_; }

    dr4::Vec2f GetSize() const override {
        if (!window_) return size_;
        int w = 0, h = 0;
        SDL_GetWindowSize(window_, &w, &h);
        return dr4::Vec2f(static_cast<float>(w), static_cast<float>(h));
    }

    void SetSize(dr4::Vec2f size) override {
        size_ = size;
        if (window_) {
            SDL_SetWindowSize(window_,
                              static_cast<int>(SDL_ceilf(size.x)),
                              static_cast<int>(SDL_ceilf(size.y)));
        }
    }

    void Open() override {
        if (is_open_) return;

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());
        }

        if (!SDL_CreateWindowAndRenderer(
                title_.c_str(),
                static_cast<int>(size_.x),
                static_cast<int>(size_.y),
                SDL_WINDOW_RESIZABLE,
                &window_, &renderer_)) {
            std::string err = SDL_GetError();
            SDL_Quit();
            throw std::runtime_error(err);
        }

        if (!TTF_Init()) {
            std::string err = SDL_GetError();
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            renderer_ = nullptr;
            SDL_Quit();
            throw std::runtime_error(err);
        }

        text_engine_ = TTF_CreateRendererTextEngine(renderer_);
        if (!text_engine_) {
            std::string err = SDL_GetError();
            TTF_Quit();
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            window_ = nullptr;
            renderer_ = nullptr;
            SDL_Quit();
            throw std::runtime_error(err);
        }

        is_open_ = true;
    }

    bool IsOpen() const override { return is_open_; }

    void Close() override {
        if (!is_open_) return;

        if (text_engine_) {
            TTF_DestroyRendererTextEngine(text_engine_);
            text_engine_ = nullptr;
        }
        TTF_Quit();

        if (renderer_) { SDL_DestroyRenderer(renderer_); renderer_ = nullptr; }
        if (window_)   { SDL_DestroyWindow(window_);     window_   = nullptr; }
        SDL_Quit();

        is_open_ = false;
    }

    void Clear(dr4::Color color) override {
        ensureOpen_();
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, color.r, color.g, color.b, color.a);
        SDL_RenderClear(renderer_);
    }

    void Draw(const dr4::Texture &texture, dr4::Vec2f) override {
        ensureOpen_();

        const SwuixTexture *sw = static_cast<const SwuixTexture*>(&texture);
        if (!sw || !sw->GetSDLTexture()) return;

        float tw = 0, th = 0;
        SDL_GetTextureSize(sw->GetSDLTexture(), &tw, &th);

        dr4::Vec2f pos = texture.GetPos();
        SDL_FRect dst{ pos.x, pos.y, tw, th };
        SDL_SetTextureBlendMode(sw->GetSDLTexture(), SDL_BLENDMODE_BLEND);
        SDL_RenderTexture(renderer_, sw->GetSDLTexture(), nullptr, &dst);
    }

    void Display() override {
        ensureOpen_();
        SDL_RenderPresent(renderer_);
    }

    double GetTime() override {
        return static_cast<double>(SDL_GetTicks()) / 1000.0;
    }

    dr4::Texture   *CreateTexture()   override { return new SwuixTexture(renderer_, text_engine_); }
    dr4::Image     *CreateImage()     override { return new SwuixImage(); }
    dr4::Font      *CreateFont()      override { return new SwuixFont(); }
    dr4::Line      *CreateLine()      override { return new SwuixLine(); }
    dr4::Circle    *CreateCircle()    override { return new SwuixCircle(); }
    dr4::Rectangle *CreateRectangle() override { return new SwuixRectangle(); }
    dr4::Text      *CreateText()      override { return new SwuixText(); }

    void StartTextInput() override {
        ensureOpen_();
        SDL_StartTextInput(window_);
    }

    void StopTextInput() override {
        ensureOpen_();
        SDL_StopTextInput(window_);
    }

    std::optional<dr4::Event> PollEvent() override {
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
            out.mouseButton.button = MapSDLMouseButton(e.button.button);
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
            out.mouseWheel.deltaX = e.wheel.x;
            out.mouseWheel.deltaY = e.wheel.y;
            out.mouseWheel.pos    = dr4::Vec2f(mx, my);
            return out;
        }
        case SDL_EVENT_TEXT_INPUT: {
            out.type = dr4::Event::Type::TEXT_EVENT;
            out.text.unicode = e.text.text;
            return out;
        }

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            out.type = (e.type == SDL_EVENT_KEY_DOWN)
                     ? dr4::Event::Type::KEY_DOWN
                     : dr4::Event::Type::KEY_UP;
            out.key.sym  = MapSDLKey(e.key.key);
            out.key.mods = MapSDLModToDR4(SDL_GetModState());
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

private:
    SDL_Window     *window_;
    SDL_Renderer   *renderer_;
    TTF_TextEngine *text_engine_;

    dr4::Vec2f   size_;
    std::string  title_;
    bool         is_open_;

    void ensureOpen_() const {
        if (!is_open_ || !window_ || !renderer_) {
            throw std::runtime_error("SwuixWindow: window not open");
        }
    }
};
