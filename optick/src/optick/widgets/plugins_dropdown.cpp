#include "plugins_dropdown.hpp"

#include "desktop.hpp"
#include "canvas.hpp"
#include "canvas_toolbar.hpp"
#include "tool_group.hpp"
#include "widgets/popup_menu.hpp"

static PopupMenu *findExistingPopup(Desktop *root) {
    if (!root) return nullptr;
    for (Widget *c : root->children) {
        if (auto *pm = dynamic_cast<PopupMenu*>(c))
            return pm;
    }
    return nullptr;
}

static bool isEssentialsGroup(const ToolGroupDesc &g) {
    return g.label == "Essential tools"
        || g.label == "Essentials"
        || g.label == "pp.tools.essentials";
}

void SelectToolPtrAction::apply(void *, Widget *) {
    if (!root_ || !tool_) return;

    auto *canvas = root_->findDescendant<Canvas>();
    if (!canvas) return;

    const auto &ts = canvas->tools();
    for (size_t i = 0; i < ts.size(); ++i) {
        if (ts[i].get() == tool_) {
            canvas->setActiveTool(i);
            canvas->requestRedraw();
            return;
        }
    }
}

void TogglePluginsDropdownAction::apply(void *, Widget *target) {
    if (!root_ || !target) return;

    if (auto *existing = findExistingPopup(root_)) {
        existing->destroy();
        return;
    }

    const bool canvasActive = root_->findDescendant<Canvas>() != nullptr;

    std::vector<MenuItemDesc> items;

    if (!canvasActive) {
        items.push_back({ "(Plugin tools available in Annotate mode)", new BtnCallbackAction(nullptr), false });

        Rect2f tf = target->absoluteFrame();
        Vec2f pos { tf.pos.x, tf.pos.y + tf.size.y };
        Rect2f pf { pos.x, pos.y, 240.0f, 10.0f };

        auto *popup = new PopupMenu(items, pf, root_, target->state);
        root_->prependChild(popup);
        root_->requestRedraw();
        return;
    }

    auto *toolbar = root_->findDescendant<CanvasToolBar>();
    if (!toolbar) return;

    const auto &groups = toolbar->groups();

    for (const auto &g : groups) {
        if (isEssentialsGroup(g)) continue;

        items.push_back({ "-- " + g.label + " --", new BtnCallbackAction(nullptr), false });

        for (pp::Tool *t : g.tools) {
            if (!t) continue;

            std::string label = std::string(t->Name()) + std::string("");
            if (!t->Icon().empty()) {
                label = std::string(t->Icon()) + "  " + label;
            }

            items.push_back({ "  " + label, new SelectToolPtrAction(root_, t), true });
        }
    }

    if (items.empty()) {
        items.push_back({ "(No plugin tools)", new BtnCallbackAction(nullptr), false });
    }

    Rect2f tf = target->absoluteFrame();
    Vec2f pos { tf.pos.x, tf.pos.y + tf.size.y };

    Rect2f pf { pos.x, pos.y, 220.0f, 10.0f };

    auto *popup = new PopupMenu(items, pf, root_, target->state);
    root_->prependChild(popup);
    root_->requestRedraw();
}
