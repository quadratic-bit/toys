#pragma once
#include <pp/tool.hpp>

#include <swuix/widgets/button.hpp>
#include <swuix/widget.hpp>

class Desktop;

class SelectToolPtrAction final : public Action {
    Desktop  *root_;
    pp::Tool *tool_;

public:
    SelectToolPtrAction(Desktop *r, pp::Tool *t)
        : root_(r), tool_(t) {}

    void apply(void *, Widget *) override;
};

class TogglePluginsDropdownAction final : public Action {
    Desktop *root_;

public:
    TogglePluginsDropdownAction(Desktop *r) : root_(r) {}
    void apply(void *, Widget *target) override;
};
