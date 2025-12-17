#include "widgets/canvas_toolbar.hpp"

DispatchResult Canvas::onKeyDown(DispatcherCtx ctx, const KeyDownEvent *e) {
    if (state->getFocus() != this) return PROPAGATE;
    dr4::Event::KeyEvent kev{};
    kev.sym = (dr4::KeyCode)e->keycode;
    kev.mods = e->mods;

    if (activeTool_ && e->keycode == dr4::KEYCODE_ESCAPE) {
        activeTool_->OnEnd();
        activeTool_ = nullptr;
        requestRedraw();
        auto tb = parent->findDescendant<CanvasToolBar>();
        if (tb) tb->requestRedrawAll();
        return CONSUME;
    }

    if ((e->mods & dr4::KEYMOD_CTRL) && e->keycode == dr4::KEYCODE_P) {
        parent->onKeyDown(ctx, e);
    }

    if (activeTool_) {
        requestRedraw();
        if (activeTool_->OnKeyDown(kev)) {
            return CONSUME;
        }
    }

    return PROPAGATE;
}
