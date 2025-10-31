#pragma once
#include <cassert>
#include <swuix/widgets/tall_view.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/widgets/container.hpp>
#include <swuix/widgets/button.hpp>

#include "swuix/window/common.hpp"
#include "trace/objects.hpp"
#include "objview.hpp"

class ObjectPreview;

class Select : public Action {
    Object *target;
    ObjectView *view;

public:
    Select(Object *o, ObjectView *v) : target(o), view(v) {}

    void apply(void *, Widget *);
};

class ObjectPreview : public WidgetContainer {
    const Object &obj;

public:
    ObjectPreview(Object *o, ObjectView *v, Rect2F rect, Widget *parent_, State *state_)
            : Widget(rect, parent_, state_), WidgetContainer(rect, parent_, state_),
            obj(*o) {
        Button *select_btn = new Button(frect(rect.w - 75, 5, 70, rect.h - 10), NULL, "Select", state_, new Select(o, v));
        Widget *objs[] = { select_btn };
        this->append_children(Widget::makeChildren(objs));
    }

	const char *title() const {
		return obj.name.c_str();
	}

	void render(Window *window, float off_x, float off_y) {
        if (obj.selected()) {
            //                                       TODO: move into a constant (soft-selection)
            window->clear_rect(frame, off_x, off_y, OKLabDarken(RGB(CLR_SURFACE_2), 0.18));
        } else {
            window->clear_rect(frame, off_x, off_y, RGB(CLR_SURFACE_2));
        }

		window->text(title(), frame.x + off_x + 5, frame.y + off_y + 5, RGB(CLR_TEXT_STRONG));

		window->outline(frame, off_x, off_y, RGB(CLR_BORDER), 2);
	}
};

class ObjectsList : public TallView {
    const std::vector<Object*> &objects;

public:
	ObjectsList(const std::vector<Object*> &objects_, ObjectView *view, Rect2F rect, Rect2F clip, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), TallView(rect, clip, parent_, state_), objects(objects_) {
        std::vector<Widget*> cards = std::vector<Widget*>();
        cards.reserve(cards.size());
        for (size_t i = 0; i < objects.size(); ++i) {
            ObjectPreview *obj = new ObjectPreview(objects[i], view, frect(5, 5 + 35 * i, frame.w - 20, 30), NULL, state);
            cards.push_back(obj);
        }

		this->append_children(cards);

        frame.h = 5 + 35 * objects.size();
        refresh_layout();
	}

	const char *title() const {
		return "Objects";
	}

	void render(Window *window, float off_x, float off_y) {
		window->clear_rect(viewport, off_x, off_y, RGB(CLR_SURFACE_2));
		window->outline(viewport, off_x, off_y, RGB(CLR_BORDER), 2);
	}
};

void Select::apply(void *, Widget *) {
    bool toggled = target->toggleSelect();
    if (toggled) {
        view->populate(target);
    } else {
        // TODO: multiple selection
    }
}
