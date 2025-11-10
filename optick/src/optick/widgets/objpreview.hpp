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
        //if (obj->selected()) {
        //    //                                       TODO: move into a constant (soft-selection)
        //    window->clear_rect(frame, off_x, off_y, OKLabDarken(RGB(CLR_SURFACE_2), 0.18));
        //} else {
        //    window->clear_rect(frame, off_x, off_y, RGB(CLR_SURFACE_2));
        //}

		//window->text(title(), frame.x + off_x + 5, frame.y + off_y + 5, RGB(CLR_TEXT_STRONG));

		//window->outline(frame, off_x, off_y, RGB(CLR_BORDER), 2);
	}
};

class ObjectsList final : public TallView {
    const std::vector<Object*> &objects;
    ObjectView *view;
    ObjectPreview *selected;

public:
	ObjectsList(const std::vector<Object*> &objects_, ObjectView *v, Rect2f f, Vec2f clip, Widget *p, State *s)
			: Widget(f, p, s), TallView(f, clip, p, s), objects(objects_), view(v), selected(NULL) {
        for (size_t i = 0; i < objects.size(); ++i) {
            ObjectPreview *obj = new ObjectPreview(objects[i], this, {5, static_cast<float>(5 + 35 * i), frame().size.x - 20, 30}, NULL, state);
            this->appendChild(obj);
        }

        texture->SetSize({texture->GetWidth(), static_cast<float>(5 + 35 * objects.size())});
        requestLayout();
        requestRedraw();
	}

    void toggleSelect(ObjectPreview *preview) {
        bool toggled = preview->obj->toggleSelect();

        if (toggled) {
            if (selected) selected->obj->unselect();
            view->populate(preview->obj);
            selected = preview;
        } else {
            assert(selected == preview);
            assert(selected != NULL);
            selected->obj->unselect();
            view->unpopulate();
            selected = NULL;
        }
    }

	const char *title() const override {
		return "Objects";
	}

	void draw() override {
        texture->Clear(dr4::Color(CLR_BORDER, 255));
        Rect2f f = frame();
        Rectangle r{
            Rect2f(2, 2, f.size.x - 4, f.size.y - 4),
            Color(CLR_SURFACE_2, 255)
        };
        texture->Draw(r);
	}
};

inline void Select::apply(void *, Widget *) {
    list->toggleSelect(preview);
}
