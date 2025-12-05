#include "canvas_toolbar.hpp"

void SelectToolAction::apply(void *, Widget *) {
    if (!canvas_) return;
    canvas_->setActiveTool(toolIndex_);
    canvas_->requestRedraw();
    toolbar_->requestRedraw();
    toolbar_->NotifyToolSwitched(toolIndex_);
}
