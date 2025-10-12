#pragma once
#include <swuix/widget.hpp>

class WidgetContainer : public virtual Widget {
protected:
	std::vector<Widget*> children;
public:
	WidgetContainer(FRect f, Widget *par, State *st) : Widget(f, par, st) {}
	WidgetContainer(FRect f, Widget *par, std::vector<Widget*> children_, State *st)
		: Widget(f, par, st), children(children_) {}

	const char *title() const {
		return "Container";
	}

	void append_children(const std::vector<Widget*> &new_children) {
		children.reserve(children.size() + new_children.size());
		for (size_t ch = 0; ch < new_children.size(); ++ch) {
			Widget *child = new_children[ch];
			child->parent = this;
			children.push_back(child);
		}
	}

	DispatchResult broadcast(DispatcherCtx ctx, Event *e, bool reversed=false) {
		ctx.clip(get_viewport());

		DispatcherCtx local_ctx = ctx.with_offset(frame);

		if (reversed) {
			if (e->deliver(ctx, this) == CONSUME) return CONSUME;

			for (int i = (int)children.size() - 1; i >= 0; --i) {
				if (children[i]->broadcast(local_ctx, e, true) == CONSUME) {
					return CONSUME;
				}
			}

			return PROPAGATE;
		}

		for (size_t i = 0; i < children.size(); ++i) {
			if (children[i]->broadcast(local_ctx, e) == CONSUME) {
				return CONSUME;
			}
		}

		return e->deliver(ctx, this);
	}
};
