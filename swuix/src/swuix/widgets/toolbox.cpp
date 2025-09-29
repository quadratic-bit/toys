#include "toolbox.hpp"
#include "../window/window.hpp"

void ToolboxWidget::render_body(Window *window, int off_x, int off_y) {
	// body
	window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
	window->outline(frame, off_x, off_y, 2);

	// children
	HandledContainer::render_body(window, off_x, off_y);
}
