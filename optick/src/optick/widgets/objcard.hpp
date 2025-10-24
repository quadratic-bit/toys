#pragma once
#include <swuix/widgets/scrollbar.hpp>

#include "trace/objects.hpp"

class ObjectCard : public Widget {
    const Object &obj;

public:
    ObjectCard(const Object &obj_, Rect2F rect, Widget *parent_, State *state_)
        : Widget(rect, parent_, state_), obj(obj_) {}

	const char *title() const {
		return obj.name.c_str();
	}

	void render(Window *window, float off_x, float off_y) {
		window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);

		window->text(title(), frame.x + off_x + 5, frame.y + off_y + 5);

		window->outline(frame, off_x, off_y, 2);
	}
};

class ObjectsView : public TallView {
    const std::vector<Object*> &objects;

public:
	ObjectsView(const std::vector<Object*> &objects_, Rect2F rect, Rect2F clip, Widget *parent_, State *state_)
			: Widget(rect, parent_, state_), TallView(rect, clip, parent_, state_), objects(objects_) {
        std::vector<Widget*> cards = std::vector<Widget*>();
        cards.reserve(cards.size());
        for (size_t i = 0; i < objects.size(); ++i) {
            ObjectCard *obj = new ObjectCard(*objects[i], frect(5, 5 + 35 * i, frame.w - 20, 30), NULL, state);
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
