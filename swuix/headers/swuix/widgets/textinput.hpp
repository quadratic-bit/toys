#pragma once
#include <SDL3/SDL_stdinc.h>
#include <string>

#include <swuix/state.hpp>
#include <swuix/widgets/focusable.hpp>

class TextInput : public virtual FocusableWidget {
    void remove_last_character() {
        if (value.empty()) return;

        ssize_t i = (ssize_t)value.size() - 1;

        // continuation bytes are of form 10xxxxxx
        while (i >= 0 && (static_cast<unsigned char>(value[i]) & 0xC0) == 0x80)
            --i;

        if (i >= 0) value.erase(i);
        else value.clear();
    }

    void append_codepoint_utf8(uint32_t cp) {
        if (cp <= 0x7F) {
            value.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7FF) {
            value.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
            value.push_back(static_cast<char>(0x80 |  (cp       & 0x3F)));
        } else if (cp <= 0xFFFF) {
            // surrogate halves
            if (cp >= 0xD800 && cp <= 0xDFFF) return;
            value.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
            value.push_back(static_cast<char>(0x80 | ((cp >> 6)  & 0x3F)));
            value.push_back(static_cast<char>(0x80 |  (cp        & 0x3F)));
        } else if (cp <= 0x10FFFF) {
            value.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
            value.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            value.push_back(static_cast<char>(0x80 | ((cp >> 6)  & 0x3F)));
            value.push_back(static_cast<char>(0x80 |  (cp        & 0x3F)));
        }
    }

    void append(uint32_t cp) {
        if (cp == 0) return;

        if (cp < 0x20 && cp != '\t') return;

        // surrogates and out-of-Unicode range
        if (cp > 0x10FFFF) return;
        if (cp >= 0xD800 && cp <= 0xDFFF) return;

        append_codepoint_utf8(cp);
    }

protected:
    std::string value;

public:
    TextInput(Rect2F frame_, Widget *parent_, State *state_)
        : Widget(frame_, parent_, state_) {}

    const std::string &getText() const {
        return value;
    }

    virtual DispatchResult on_key_down(DispatcherCtx, const KeyDownEvent *e) {
        if (state->mouse.focus != this) return PROPAGATE;

        if (e->keycode == SDLK_BACKSPACE) {
            remove_last_character();
        } else {
            append(e->keycode);
        }

        return CONSUME;
    }
};
