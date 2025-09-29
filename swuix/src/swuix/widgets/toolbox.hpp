#pragma once
#include "handle.hpp"

class ToolboxWidget : public HandledContainer {
public:
	ToolboxWidget(SDL_FRect rect, Widget *parent_, State *state_)
		: Widget(rect, parent_, state_), HandledContainer(rect, parent_, state_) {}

	ToolboxWidget(SDL_FRect rect, Widget *parent_, std::vector<Widget*> children_, State *state_)
			: Widget(rect, parent_, state_), HandledContainer(rect, parent_, children_, state_) {}

	void render_body(Window *window, int off_x, int off_y);
};
