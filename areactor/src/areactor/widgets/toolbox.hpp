#pragma once
#include "handle.hpp"

class ToolboxWidget : public HandledWidget, public WidgetContainer {
public:
	ToolboxWidget(SDL_FRect rect, Widget *parent_, State *state_)
		: Widget(rect, parent_, state_), HandledWidget(rect, parent_, state_), WidgetContainer(rect, parent_, state_) {}

	ToolboxWidget(SDL_FRect rect, Widget *parent_, std::vector<Widget*> children_, State *state_)
			: Widget(rect, parent_, state_), HandledWidget(rect, parent_, state_), WidgetContainer(rect, parent_, children_, state_) {}

	size_t child_count() const {
		return HandledWidget::child_count() + WidgetContainer::child_count();
	}

	Widget *child_at(size_t i) const {
		if (i == child_count() - 1) {
			return HandledWidget::child_at(0);
		}
		return WidgetContainer::child_at(i);
	}

	void render(Window *window, int off_x, int off_y);

	void render_body(Window *window, int off_x, int off_y);
};
