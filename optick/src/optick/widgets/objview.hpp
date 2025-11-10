#pragma once
#include <swuix/widgets/button.hpp>
#include <swuix/widgets/titled.hpp>
#include <swuix/widgets/textinput.hpp>

#include "swuix/common.hpp"
#include "swuix/window/common.hpp"
#include "trace/objects.hpp"

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
        Rectangle *r = outline(state->window, frame(), 1, {CLR_BORDER});
        texture->Draw(*r);

        Text *t = textAligned(state->window, property->name().c_str(), {5,texture->GetHeight() / 2}, Color(CLR_TEXT_STRONG), state->appfont);
        texture->Draw(*t);

        // TODO: TA_RIGHT
        t = textAligned(state->window, property->serialize().c_str(), {texture->GetWidth() - 10, texture->GetHeight() / 2}, Color(CLR_TEXT_STRONG), state->appfont);
        texture->Draw(*t);
    }
};

class ObjectViewPropertyList final : public Widget {
    Object *obj;
    FieldList fields;

public:
    ObjectViewPropertyList(Object *o, Rect2f f, Widget *p, State *s)
            : Widget(f, p, s), obj(o) {
        obj->mat->collectFields(fields);            // fill member 'fields'
        Text *t = textAligned(state->window, "Материал", {1, 1}, Color(CLR_TEXT_STRONG), state->appfont, 16, HAlign::LEFT, dr4::Text::VAlign::TOP);
        Vec2f offset = t->GetBounds();
        for (size_t i = 0; i < fields.size(); ++i) {
            ObjectViewProperty *property = new ObjectViewProperty(fields[i], {0, static_cast<float>(i * 35) + offset.y + 3, f.size.x, 35}, NULL, s);
            appendChild(property);
        }
    }

    const char *title() const override {
        return "ObjectView property list";
    }

    void draw() override {
        Text *t = textAligned(state->window, "Материал", {1, 1}, Color(CLR_TEXT_STRONG), state->appfont, 16, HAlign::LEFT, dr4::Text::VAlign::TOP);
        texture->Draw(*t);
    }
};

class ObjectPreview;

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

    void populate(ObjectPreview *prev);

	void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);
	}
};
