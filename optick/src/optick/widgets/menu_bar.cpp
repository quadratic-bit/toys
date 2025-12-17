#include "desktop.hpp"
#include "renderer.hpp"

#include "widgets/popup_menu.hpp"
#include "theme_picker_action.hpp"
#include "canvas.hpp"

static PopupMenu *findExistingPopup(Desktop *root) {
    if (!root) return nullptr;
    for (Widget *c : root->children) {
        if (auto *pm = dynamic_cast<PopupMenu*>(c))
            return pm;
    }
    return nullptr;
}

void LaunchObjView::apply(void *, Widget *) {
    root->createView();
}

void ToggleRender::apply(void *, Widget *) {
    Renderer *r = root->findChild<Renderer>();
    if (r) r->toggleRender();
}

void ScreenshotAction::apply(void *, Widget *) {
    if (!root_) return;
    if (auto *r = root_->findDescendant<Renderer>()) {
        r->captureScreenshot();
    }
}

void TogglePropertiesPanelAction::apply(void *, Widget *) {
    if (root_) root_->togglePropertiesView();
}

void ToggleControlsPanelAction::apply(void *, Widget *) {
    if (root_) root_->toggleControlsPanel();
}

void ToggleFileDropdownAction::apply(void *, Widget *target) {
    if (!root_ || !target) return;

    if (auto *existing = findExistingPopup(root_)) {
        existing->destroy();
    }

    const bool canvasActive = root_->findDescendant<Canvas>() != nullptr;

    std::vector<MenuItemDesc> items;
    items.push_back({ "Screenshot", new ScreenshotAction(root_),        true });
    items.push_back({ "Annotate",   new ToggleRender(root_),            true });
    items.push_back({ "Theme",      new ToggleThemePickerAction(root_), canvasActive });

    Rect2f tf = target->absoluteFrame();
    Vec2f pos { tf.pos.x, tf.pos.y + tf.size.y };
    Rect2f pf { pos.x, pos.y, 200.0f, 10.0f };

    auto *popup = new PopupMenu(items, pf, root_, target->state);
    root_->prependChild(popup);
    root_->requestRedraw();
}

void ToggleViewDropdownAction::apply(void *, Widget *target) {
    if (!root_ || !target) return;

    if (auto *existing = findExistingPopup(root_)) {
        existing->destroy();
    }

    std::vector<MenuItemDesc> items;
    items.push_back({ "Properties", new TogglePropertiesPanelAction(root_), true });
    items.push_back({ "Controls",   new ToggleControlsPanelAction(root_),   true });

    Rect2f tf = target->absoluteFrame();
    Vec2f pos { tf.pos.x, tf.pos.y + tf.size.y };
    Rect2f pf { pos.x, pos.y, 200.0f, 10.0f };

    auto *popup = new PopupMenu(items, pf, root_, target->state);
    root_->prependChild(popup);
    root_->requestRedraw();
}
