#pragma once
#include <cassert>
#include <swuix/widgets/tall_view.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/widgets/button.hpp>

#include "trace/objects.hpp"
#include "objview.hpp"

class ObjectPreview;
class ObjectsList;

class Select : public Action {
    ObjectPreview *preview;
    ObjectsList *list;

public:
    Select(ObjectPreview *p, ObjectsList *l) : preview(p), list(l) {}

    void apply(void *, Widget *);
};

class ObjectPreview final : public Widget {
    friend class ObjectsList;
    friend class ObjectView;
    friend class ObjectViewName;
    Object *obj;

public:
    ObjectPreview(Object *o, ObjectsList *l, Rect2f f, Widget *p, State *s)
            : Widget(f, p, s), obj(o) {
        Button *select_btn = new Button({f.size.x - 75, 5, 70, f.size.y - 10}, NULL, "Select", state, new Select(this, l));
        this->appendChild(select_btn);
    }

	const char *title() const override {
		return obj->name.c_str();
	}

	void draw() override {
        Rect2f f = frame();
        Rectangle *r;
        if (obj->selected()) {
            // TODO: move into a constant (soft-selection)
            const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.18);
            r = rectBorder(state->window, f, {d.r, d.g, d.b}, 2, {CLR_BORDER});
        } else {
            r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        }
        texture->Draw(*r);

        Text *t = textAligned(state->window, title(), {5, f.size.y / 2}, Color(CLR_TEXT_STRONG), state->appfont);
        texture->Draw(*t);
	}
};

class Desktop;

class ObjectsList final : public TallView {
    const std::vector<Object*> &objects;
    Desktop *root;
    ObjectPreview *selected;

public:
	ObjectsList(const std::vector<Object*> &objects_, Desktop *d, Rect2f f, Vec2f clip, Widget *p, State *s)
			: Widget(f, p, s), TallView(f, clip, p, s), objects(objects_), root(d), selected(NULL) {
        for (size_t i = 0; i < objects.size(); ++i) {
            ObjectPreview *obj = new ObjectPreview(objects[i], this, {5, static_cast<float>(5 + 35 * i), frame().size.x - 20, 30}, NULL, state);
            this->appendChild(obj);
        }

        texture->SetSize({texture->GetWidth(), static_cast<float>(5 + 35 * objects.size())});
        requestLayout();
        requestRedraw();
	}

    void toggleSelect(ObjectPreview *preview);

	const char *title() const override {
		return "Objects";
	}

	void draw() override {
        texture->Clear({CLR_SURFACE_2});
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);
	}
};

class ObjectViewName final : public TextInput {
    ObjectPreview *prev;

public:
    ObjectViewName(ObjectPreview *preview, Rect2f f, Widget *p, State *s)
            : Widget(f, p, s),
            FocusableWidget(f, p, s),
            TextInput(f, p, s), prev(preview) {
        setText(preview->obj->name);
    }

    const char *title() const override {
        return "ObjectView name edit";
    }

    void onValueChange() override {
        prev->obj->name = value;
        prev->requestRedraw();
    }

    void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);

        Text *t = textAligned(state->window, getText().c_str(), {5, f.size.y / 2}, Color(CLR_TEXT_STRONG), state->appfont);
        texture->Draw(*t);
    }
};

inline void ObjectView::populate(ObjectPreview *prev) {
    unpopulate();

    Object *obj = prev->obj;

    ObjectViewName *objname = new ObjectViewName(prev, {5, 5, 125, 24}, NULL, state);
    this->appendChild(objname);

    Rect2f f = frame();
    Rect2f list_frame {
        8,
        60,
        f.size.x - 16,
        f.size.y - 68
    };

    ObjectViewPropertyList *objprops = new ObjectViewPropertyList(obj, list_frame, nullptr, state);
    this->appendChild(objprops);

    requestRedraw();
}

inline void Select::apply(void *, Widget *) {
    list->toggleSelect(preview);
}
