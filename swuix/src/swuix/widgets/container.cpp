#include <swuix/widgets/container.hpp>

void WidgetContainer::render(Window *window, int off_x, int off_y) {
	const int n = (int)child_count();
	for (int i = n - 1; i >= 0; --i) {
		Widget *ch = child_at(i);
		ch->render(window, frame.x + off_x, frame.y + off_y);
	}
}
