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

class CanvasToolBar;

class SelectToolAction final : public Action {
    Canvas        *canvas_;
    CanvasToolBar *toolbar_;
    size_t         toolIndex_;

public:
    SelectToolAction(Canvas *c, CanvasToolBar *tb, size_t idx)
        : canvas_(c), toolbar_(tb), toolIndex_(idx) {}

    void apply(void *, Widget *) override;
};

class ToolButton final : public Button {
    Canvas *canvas_;
    size_t  index_;

    bool isActive() const {
        if (!canvas_) return false;
        const auto &ts = canvas_->tools();
        if (index_ >= ts.size()) return false;
        return canvas_->activeTool() == ts[index_].get();
    }

public:
    ToolButton(Rect2f f, Widget *p, const char *l, State *s,
               Action *a, Canvas *c, size_t idx)
        : Button(f, p, l, s, a), canvas_(c), index_(idx) {}

    void draw_idle() override {
        Rect2f f = frame();
        Rectangle *r;

        if (isActive()) {
            const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), 0.18);
            r = rectBorder(state->window, f, {d.r, d.g, d.b}, 2, {CLR_PRIMARY});
        } else {
            r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        }
        texture->Draw(*r);
    }

    void draw_hover() override {
        Rect2f f = frame();
        Rectangle *r;

        const float amt = isActive() ? 0.22f : 0.08f;
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), amt);
        r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);

        r = outline(state->window, f, 1, {CLR_BORDER});
        texture->Draw(*r);
    }

    void draw_press() override {
        Rect2f f = frame();
        Rectangle *r;

        const float amt = isActive() ? 0.28f : 0.12f;
        const RGBu8 d = OKLabDarken(RGB(CLR_SURFACE_2), amt);
        r = rectFill(state->window, f, {d.r, d.g, d.b});
        texture->Draw(*r);

        r = outline(state->window, f, 1, {CLR_BORDER});
        texture->Draw(*r);
    }
};

class CanvasToolBar final : public TitledWidget {
    Canvas *canvas_;
    std::vector<ToolGroupDesc> groups_;

    std::vector<Widget*> toolButtonsByIndex_;
    size_t lastActiveIndex_ = static_cast<size_t>(-1);

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

    const std::vector<ToolGroupDesc> &groups() const { return groups_; }

    const char *title() const override {
        return "Tools";
    }

    void draw() override {
        Rect2f f = frame();

        texture->Clear({CLR_SURFACE_2});
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);

        for (size_t i = 0; i < groups_.size() && i < groupLabelY_.size(); ++i) {
            float y = groupLabelY_[i];
            Text *t = textAligned(
                state->window,
                groups_[i].label.c_str(),
                {5.0f, y + 8.0f},
                Color(CLR_TEXT_STRONG),
                state->appfont
            );
            texture->Draw(*t);
        }
    }

    void requestRedrawAll() {
        for (auto btn : toolButtonsByIndex_) {
            btn->requestRedraw();
        }
    }

    void RegisterToolButton(size_t idx, Widget *btn) {
        if (idx >= toolButtonsByIndex_.size())
            toolButtonsByIndex_.resize(idx + 1, nullptr);
        toolButtonsByIndex_[idx] = btn;
    }

    void NotifyToolSwitched(size_t newIdx) {
        if (lastActiveIndex_ != static_cast<size_t>(-1) &&
            lastActiveIndex_ < toolButtonsByIndex_.size() &&
            toolButtonsByIndex_[lastActiveIndex_]) {
            toolButtonsByIndex_[lastActiveIndex_]->requestRedraw();
        }

        if (newIdx < toolButtonsByIndex_.size() &&
            toolButtonsByIndex_[newIdx]) {
            toolButtonsByIndex_[newIdx]->requestRedraw();
        }

        lastActiveIndex_ = newIdx;
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

                auto *act = new SelectToolAction(canvas_, this, globalToolIndex);

                Button *btn = new ToolButton(
                    bf, this, label, state, act, canvas_, globalToolIndex
                );

                appendChild(btn);

                RegisterToolButton(globalToolIndex, btn);

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
