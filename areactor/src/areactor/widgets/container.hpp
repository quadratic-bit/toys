#pragma once
#include "widget.hpp"

class WidgetContainer : public virtual Widget {
	std::vector<Widget*> children;
public:
	WidgetContainer(SDL_FRect f, Widget *par, State *st) : Widget(f, par, st) {}
	WidgetContainer(SDL_FRect f, Widget *par, std::vector<Widget*> children_, State *st)
		: Widget(f, par, st), children(children_) {}

	void append_children(const std::vector<Widget*> &new_children) {
		children.reserve(children.size() + new_children.size());
		for (size_t ch = 0; ch < new_children.size(); ++ch) {
			Widget *child = new_children[ch];
			child->parent = this;
			children.push_back(child);
		}
	}

	size_t child_count() const {
		return children.size();
	}

	Widget *child_at(size_t i) const {
		return children[i];
	}

	void render(Window *window, int off_x, int off_y);
};
