#include "desktop.hpp"
#include "renderer.hpp"

void LaunchObjView::apply(void *, Widget *) {
    root->createView();
}

void ToggleRender::apply(void *, Widget *) {
    Renderer *r = root->findChild<Renderer>();
    if (r) r->toggleRender();
}
