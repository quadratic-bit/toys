#pragma once
#include <vector>

#include <swuix/widget.hpp>
#include <swuix/widgets/button.hpp>
#include <swuix/widgets/titled.hpp>

#include <swuix/common.hpp>
#include <swuix/state.hpp>

#include <pp/tool.hpp>
#include "canvas.hpp"
#include "tool_group.hpp"

class SelectToolAction final : public Action {
    Canvas *canvas_;
    size_t toolIndex_;

public:
    SelectToolAction(Canvas *c, size_t idx)
        : canvas_(c), toolIndex_(idx) {}

    void apply(void *, Widget *) override {
        if (!canvas_) return;
        canvas_->setActiveTool(toolIndex_);
        canvas_->requestRedraw();
    }
};

class CanvasToolBar final : public TitledWidget {
    Canvas *canvas_;
    std::vector<ToolGroupDesc> groups_;

    // Y coordinate for each group header label
    std::vector<float> groupLabelY_;

public:
    CanvasToolBar(Canvas *c,
                  const std::vector<ToolGroupDesc> &groups,
                  Rect2f f,
                  Widget *p,
                  State *s)
        : Widget(f, p, s)
        , TitledWidget(f, p, s)
        , canvas_(c)
        , groups_(groups)
    {
        buildButtons();
    }

    const char *title() const override {
        return "Tools";
    }

    void draw() override {
        Rect2f f = frame();

        // bg + borders
        texture->Clear({CLR_SURFACE_2});
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);

        // group headers
        for (size_t i = 0; i < groups_.size() && i < groupLabelY_.size(); ++i) {
            float y = groupLabelY_[i];
            Text *t = textAligned(
                state->window,
                groups_[i].label.c_str(),
                {5.0f, y + 8.0f},                       // FIXME:
                Color(CLR_TEXT_STRONG),
                state->appfont
            );
            texture->Draw(*t);
        }
    }

private:
    void buildButtons() {
        groupLabelY_.clear();
        groupLabelY_.reserve(groups_.size());

        const float x            = 4.0f;
              float y            = 4.0f;
        const float w            = frame().size.x - 8.0f;
        const float labelHeight  = 18.0f;
        const float btnHeight    = 24.0f;
        const float btnSpacing   = 4.0f;
        const float groupSpacing = 10.0f;

        size_t globalToolIndex = 0;

        for (size_t gi = 0; gi < groups_.size(); ++gi) {
            groupLabelY_.push_back(y);
            y += labelHeight;

            for (pp::Tool *tool : groups_[gi].tools) {
                if (!tool) continue;

                const char *label =
                    !tool->Icon().empty()
                        ? tool->Icon().data()
                        : tool->Name().data();

                Rect2f bf { x, y, w, btnHeight };
                Button *btn = new Button(
                    bf,
                    this,
                    label,
                    state,
                    new SelectToolAction(canvas_, globalToolIndex)
                );
                appendChild(btn);

                y += btnHeight + btnSpacing;
                ++globalToolIndex;
            }

            y += groupSpacing;
        }

        texture->SetSize({texture->GetWidth(), y + 4.0f});
        requestLayout();
        requestRedraw();
    }
};
