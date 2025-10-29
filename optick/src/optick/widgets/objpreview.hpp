#pragma once
#include <cassert>
#include <swuix/widgets/tall_view.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/widgets/container.hpp>
#include <swuix/widgets/button.hpp>

#include "trace/objects.hpp"
#include "objview.hpp"

class ObjectPreview;

class Select : public Action {
    Object *target;
    State *state;
    ObjectPreview *parent;
    ObjectView *child;

public:
    Select(Object *o, ObjectPreview *p, State *s) : target(o), state(s), parent(p), child(NULL) {}

    void apply(void *, Widget *);
};

class ObjectPreview : public WidgetContainer {
    const Object &obj;

public:
    ObjectPreview(Object *obj_, Rect2F rect, Widget *parent_, State *state_)
            : Widget(rect, parent_, state_), WidgetContainer(rect, parent_, state_),
            obj(*obj_) {
        Button *select_btn = new Button(frect(rect.w - 75, 5, 70, rect.h - 10), NULL, "Select", state_, new Select(obj_, this, state_));
        Widget *objs[] = { select_btn };
        this->append_children(Widget::makeChildren(objs));
    }

	const char *title() const {
		return obj.name.c_str();
	}

	void render(Window *window, float off_x, float off_y) {
        if (obj.selected()) {
            window->clear_rect(frame, off_x, off_y, CLR_PUCE);
        } else {
            window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
        }

		window->text(title(), frame.x + off_x + 5, frame.y + off_y + 5);

		window->outline(frame, off_x, off_y, 2);
	}
};

class ObjectsList : public TallView {
    const std::vector<Object*> &objects;

public:
	ObjectsList(const std::vector<Object*> &objects_, Rect2F rect, Rect2F clip, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), TallView(rect, clip, parent_, state_), objects(objects_) {
        std::vector<Widget*> cards = std::vector<Widget*>();
        cards.reserve(cards.size());
        for (size_t i = 0; i < objects.size(); ++i) {
            ObjectPreview *obj = new ObjectPreview(objects[i], frect(5, 5 + 35 * i, frame.w - 20, 30), NULL, state);
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
		window->clear_rect(viewport, off_x, off_y, CLR_TIMBERWOLF);
		window->outline(viewport, off_x, off_y, 2);
	}
};

void Select::apply(void *, Widget *) {
    bool toggled = target->toggleSelect();
    if (toggled) {
        if (child) return;

        // FIXME: I wonder if this is fine
        ObjectPreview *preview = parent;
        ObjectsList *list = dynamic_cast<ObjectsList *>(preview->parent);
        assert(list != NULL);
        WidgetContainer *container = dynamic_cast<WidgetContainer *>(list->parent);
        assert(container != NULL);

        float w = 150, h = list->getViewport().h;
        child = new ObjectView(target, frect(list->frame.x - w, list->frame.y, w, h), NULL, state);

        container->prepend_child(child);
    } else {
        if (!child) return;
        WidgetContainer *container = dynamic_cast<WidgetContainer *>(parent->parent->parent);
        assert(container != NULL);
        container->remove_child(child);
        delete child;
        child = NULL;
    }
}
