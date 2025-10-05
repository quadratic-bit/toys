#include <swuix/widgets/handle.hpp>
#include <swuix/window/window.hpp>
#include <swuix/state.hpp>

class ToggleMinimize : public Action {
	HandledWidget *owner;
public:
	ToggleMinimize(HandledWidget *w) : owner(w) {}

	void apply(void *, Widget *) {
		owner->minimized ^= true;
	}
};

Handle::Handle(HandledWidget *parent_, State *state_)
		: Widget(handle_box(parent_->frame), parent_, state_),
		DraggableWidget(handle_box(parent_->frame), parent_, state_),
		WidgetContainer(handle_box(parent_->frame), parent_, state_) {
	static float w = 15, h = 10;
	btn_minimize = new Button(frect(frame.w - w - 5, 5, w, h), this, "-", state, new ToggleMinimize(parent_));
	Widget *btns[] = { btn_minimize };
	this->append_children(make_children(btns));
}

DispatchResult Handle::on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
	(void)e;
	if (state->mouse.state == MouseState::Dragging && is_dragging) {
		FRect new_frame = parent->frame;
		new_frame.x += ctx.mouse_rel.x - start_drag_x;
		new_frame.y += ctx.mouse_rel.y - start_drag_y;
		parent->set_frame(new_frame);
	}
	return Widget::on_mouse_move(ctx, e);
}
