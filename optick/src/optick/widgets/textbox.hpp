#pragma once
#include <swuix/widgets/textinput.hpp>

class TextBox : public TextInput {
public:
    TextBox(Rect2F frame_, Widget *parent_, State *state_)
        : Widget(frame_, parent_, state_),
        FocusableWidget(frame_, parent_, state_),
        TextInput(frame_, parent_, state_) {}

    virtual const char *title() const {
        return "TextInput";
    }

    virtual void render(Window *window, float off_x, float off_y) {
        window->text(getText().c_str(), frame.x + off_x + 5, frame.y + off_y + 5);
        window->outline(frame, off_x, off_y, 2);
    }
};
