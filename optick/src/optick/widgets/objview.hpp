#pragma once
#include <swuix/widgets/button.hpp>
#include <swuix/widgets/titled.hpp>
#include <swuix/widgets/textinput.hpp>

#include "trace/objects.hpp"

class ObjectViewName : public TextInput {
    Object *obj;

public:
    ObjectViewName(Object *o, Rect2F frame_, Widget *parent_, State *state_)
            : Widget(frame_, parent_, state_),
            FocusableWidget(frame_, parent_, state_),
            TextInput(frame_, parent_, state_), obj(o) {
        setText(obj->name);
    }

    const char *title() const {
        return "ObjectView name edit";
    }

    void on_value_change() {
        obj->name = value;
    }

    void render(Window *window, float off_x, float off_y) {
        window->text(getText().c_str(), frame.x + off_x + 5, frame.y + off_y + 1);
        window->outline(frame, off_x, off_y, 2);
    }
};

class ObjectView : public TitledContainer {
    Object *obj;

public:
    ObjectView(Object *obj_, Rect2F rect, Widget *parent_, State *state_)
            : Widget(rect, parent_, state_), TitledContainer(rect, parent_, state_),
            obj(obj_) {
        ObjectViewName *objname = new ObjectViewName(obj, frect(5, 5, 100, 24), NULL, state_);
        Widget *objs[] = { objname };
        this->append_children(Widget::makeChildren(objs));
    }

	const char *title() const {
		return obj->name.c_str();
	}

	void render(Window *window, float off_x, float off_y) {
        window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);

		window->outline(frame, off_x, off_y, 2);
	}
};
