#include "theme_picker_action.hpp"

#include "desktop.hpp"
#include "canvas.hpp"
#include "color_picker_popup.hpp"

static ColorPickerPopup *findExisting(Desktop *root) {
    if (!root) return nullptr;
    for (Widget *c : root->children) {
        if (auto *p = dynamic_cast<ColorPickerPopup*>(c))
            return p;
    }
    return nullptr;
}

void ToggleThemePickerAction::apply(void *, Widget *target) {
    if (!root_ || !target) return;

    if (auto *existing = findExisting(root_)) {
        existing->destroy();
        return;
    }

    auto *canvas = root_->findDescendant<Canvas>();
    if (!canvas) return;

    Rect2f tf = target->absoluteFrame();
    Vec2f pos { tf.pos.x, tf.pos.y + tf.size.y };

    Rect2f pf { pos.x, pos.y, 260.0f, 118.0f };

    auto *popup = new ColorPickerPopup(pf, root_, target->state, canvas);
    root_->prependChild(popup);
    root_->requestRedraw();
}
