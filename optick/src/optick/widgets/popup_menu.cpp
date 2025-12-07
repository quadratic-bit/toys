#include "popup_menu.hpp"

void CloseAfterAction::apply(void *app_state, Widget *target) {
    if (inner_) inner_->apply(app_state, target);
    if (menu_)  menu_->destroy();
}
