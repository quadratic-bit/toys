#pragma once
#include <swuix/widgets/button.hpp>

class Desktop;

class ToggleThemePickerAction final : public Action {
    Desktop *root_;
public:
    ToggleThemePickerAction(Desktop *r) : root_(r) {}
    void apply(void *, Widget *target) override;
};
