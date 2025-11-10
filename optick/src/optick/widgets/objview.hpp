#pragma once
#include <swuix/widgets/button.hpp>
#include <swuix/widgets/titled.hpp>
#include <swuix/widgets/textinput.hpp>

#include "trace/objects.hpp"

class ObjectViewName final : public TextInput {
    Object *obj;

public:
    ObjectViewName(Object *o, Rect2f f, Widget *p, State *s)
            : Widget(f, p, s),
            FocusableWidget(f, p, s),
            TextInput(f, p, s), obj(o) {
        setText(obj->name);
    }

    const char *title() const override {
        return "ObjectView name edit";
    }

    void onValueChange() override {
        obj->name = value;
    }

    void draw() override {
        texture->Clear(dr4::Color(CLR_BORDER, 255));
        Rect2f f = frame();
        Rectangle r{
            Rect2f(2, 2, f.size.x - 4, f.size.y - 4),
            Color(CLR_SURFACE_2, 255)
        };
        texture->Draw(r);
        //window->clear_rect(frame, off_x, off_y, RGB(CLR_SURFACE_2));
        //window->outline(frame, off_x, off_y, RGB(CLR_BORDER), 2);

        Text t;
        t.text = getText().c_str();
        t.color = Color(CLR_TEXT_STRONG, 255);
        // TODO: align TA_LEFT
        t.pos = {5, 1};
        t.valign = Text::VAlign::TOP;
        t.font = nullptr;
        texture->Draw(t);
        //window->text(getText().c_str(), frame.x + off_x + 5, frame.y + off_y + 1, RGB(CLR_TEXT_STRONG));
    }
};

class ObjectViewProperty final : public Widget {
    //Object *obj;
    Field *property;

public:
    ObjectViewProperty(Field *prop, Rect2f f, Widget *p, State *s)
            : Widget(f, p, s), property(prop) {}

    const char *title() const override {
        return "ObjectView property";
    }

    void draw() override {
        Text t;
        t.text = property->name().c_str();
        t.color = Color(CLR_TEXT_STRONG, 255);
        // TODO: align TA_LEFT
        t.pos = {5, texture->GetHeight() / 2};
        t.valign = Text::VAlign::MIDDLE;
        t.font = nullptr;
        texture->Draw(t);

        t.text = property->serialize().c_str();
        t.color = Color(CLR_TEXT_STRONG, 255);
        // TODO: align TA_RIGHT
        t.pos = {texture->GetWidth() - 10, texture->GetHeight() / 2};
        t.valign = Text::VAlign::MIDDLE;
        t.font = nullptr;
        texture->Draw(t);

        //window->outline(frame, off_x, off_y, RGB(CLR_BORDER), 1);
    }
};

class ObjectViewPropertyList final : public Widget {
    Object *obj;
    FieldList fields;

public:
    ObjectViewPropertyList(Object *o, Rect2f f, Widget *p, State *s)
            : Widget(f, p, s), obj(o) {
        obj->mat->collectFields(fields);            // fill member 'fields'
        for (size_t i = 0; i < fields.size(); ++i) {
            ObjectViewProperty *property = new ObjectViewProperty(fields[i], {0, static_cast<float>(i * 35), f.size.x, 35}, NULL, s);
            appendChild(property);
        }
    }

    const char *title() const override {
        return "ObjectView property list";
    }

    void draw() override {
        //// FIXME: ----> SDL_ <--- !!! prefix
        //SDL_Rect rect;

        //window->get_clip(&rect);
        //window->unclip();

        Text t;
        t.text = "Материал";
        t.color = Color(CLR_TEXT_STRONG, 255);
        // TODO: align TA_LEFT
        t.pos = {0, +20};
        t.valign = Text::VAlign::MIDDLE;
        t.font = nullptr;
        texture->Draw(t);

        //window->text("Материал", frame.x + off_x,  frame.y + off_y - 20, RGB(CLR_TEXT_STRONG));

        //window->restore_clip(&rect);
    }
};

class ObjectView final : public TitledWidget {
public:
    ObjectView(Rect2f f, Widget *p, State *s)
            : Widget(f, p, s), TitledWidget(f, p, s) {}

	const char *title() const override {
		return "Properties";
	}

    void unpopulate() {
        while (children.size() > 1) {
            delete children[children.size() - 1];
            children.pop_back();
        }
        requestRedraw();
    }

    void populate(Object *obj) {
        unpopulate();

        ObjectViewName *objname = new ObjectViewName(obj, {5, 5, 125, 24}, NULL, state);
        this->appendChild(objname);

        ObjectViewPropertyList *objprops = new ObjectViewPropertyList(obj, {5, 50, frame().size.x - 10, frame().size.y - 55}, NULL, state);
        this->appendChild(objprops);

        requestRedraw();
    }

	void draw() override {
        texture->Clear(dr4::Color(CLR_BORDER, 255));
        Rect2f f = frame();
        Rectangle r{
            Rect2f(2, 2, f.size.x - 4, f.size.y - 4),
            Color(CLR_SURFACE_2, 255)
        };
        texture->Draw(r);
        //window->clear_rect(frame, off_x, off_y, RGB(CLR_SURFACE_2));
		//window->outline(frame, off_x, off_y, RGB(CLR_BORDER), 2);
	}
};
