#pragma once
#include <SDL3/SDL_keycode.h>

#include <swuix/traits/focusable.hpp>
#include <swuix/state.hpp>

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
    TextInput(Rect2f f, Widget *p, State *s) : Widget(f, p, s) {}

    const std::string &getText() const {
        return value;
    }

    void setText(const std::string &new_value) {
        value = new_value;
    }

    virtual void onValueChange() = 0;

    DispatchResult onInput(DispatcherCtx, const InputEvent *e) override {
        if (state->getFocus() != this) return PROPAGATE;

        value.append(e->text);
        onValueChange();
        requestRedraw();

        return CONSUME;
    }

    DispatchResult onKeyDown(DispatcherCtx, const KeyDownEvent *e) override {
        if (state->getFocus() != this) return PROPAGATE;

        if (e->keycode == SDLK_BACKSPACE) {
            remove_last_character();
            onValueChange();
            requestRedraw();
            return CONSUME;
        }

        return PROPAGATE;
    }
};
