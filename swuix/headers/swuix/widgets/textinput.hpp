#pragma once
#include <SDL3/SDL_stdinc.h>
#include <string>

#include <swuix/state.hpp>
#include <swuix/traits/focusable.hpp>

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

protected:
    std::string value;

public:
    TextInput(Rect2F frame_, Widget *parent_, State *state_)
        : Widget(frame_, parent_, state_) {}

    const std::string &getText() const {
        return value;
    }

    void setText(const std::string &new_value) {
        value = new_value;
    }

    virtual void on_value_change() = 0;

    DispatchResult on_input(DispatcherCtx, const InputEvent *e) {
        if (state->get_focus() != this) return PROPAGATE;

        value.append(e->text);
        on_value_change();

        return CONSUME;
    }

    DispatchResult on_key_down(DispatcherCtx, const KeyDownEvent *e) {
        if (state->get_focus() != this) return PROPAGATE;

        if (e->keycode == SDLK_BACKSPACE) {
            remove_last_character();
            on_value_change();
            return CONSUME;
        }

        return PROPAGATE;
    }
};
