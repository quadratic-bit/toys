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

class ObjectViewProperty : public Widget {
    //Object *obj;
    Property property;

public:
    ObjectViewProperty(Property p, Rect2F frame_, Widget *parent_, State *state_)
            : Widget(frame_, parent_, state_), property(p) {}

    const char *title() const {
        return "ObjectView property";
    }

    void render(Window *window, float off_x, float off_y) {
        window->text_aligned(property.first.c_str(),  frame.x + off_x + 5,  frame.y + off_y + 15, TA_LEFT);
        window->text_aligned(property.second.c_str(), frame.x + off_x + frame.w - 10, frame.y + off_y + 15, TA_RIGHT);
        window->outline(frame, off_x, off_y, 1);
    }
};

class ObjectViewPropertyList : public WidgetContainer {
    Object *obj;

public:
    ObjectViewPropertyList(Object *o, Rect2F frame_, Widget *parent_, State *state_)
            : Widget(frame_, parent_, state_), WidgetContainer(frame_, parent_, state_), obj(o) {
        vector<Property> properties = obj->mat->getProperties();

        for (size_t i = 0; i < properties.size(); ++i) {
            ObjectViewProperty *property = new ObjectViewProperty(properties[i], frect(0, i * 35, frame.w, 35), NULL, state_);
            append_child(property);
        }
    }

    const char *title() const {
        return "ObjectView property list";
    }

    void render(Window *window, float off_x, float off_y) {
        // FIXME: ----> SDL_ <--- !!! prefix
        SDL_Rect rect;

        window->get_clip(&rect);
        window->unclip();

        window->text("Материал", frame.x + off_x,  frame.y + off_y - 20);

        window->restore_clip(&rect);
    }
};

class ObjectView : public TitledContainer {
    Object *obj;

public:
    ObjectView(Object *obj_, Rect2F rect, Widget *parent_, State *state_)
            : Widget(rect, parent_, state_), TitledContainer(rect, parent_, state_),
            obj(obj_) {
        ObjectViewName *objname = new ObjectViewName(obj, frect(5, 5, 125, 24), NULL, state_);
        ObjectViewPropertyList *objprops = new ObjectViewPropertyList(obj, frect(5, 50, frame.w - 10, frame.h - 55), NULL, state_);
        Widget *objs[] = { objname, objprops };
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
