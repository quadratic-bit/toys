#pragma once
#include <cstdio>
#include <memory>

#include "swuix/widget.hpp"
#include "swuix/state.hpp"

#include "pp/canvas.hpp"
#include "pp/tool.hpp"

class Canvas final : public Widget, public pp::Canvas {
    // owned by this canvas
    std::vector<std::unique_ptr<pp::Shape>> shapes_;
    pp::Shape *selected_{nullptr};

    // tools
    std::vector<std::unique_ptr<pp::Tool>> tools_;
    pp::Tool *activeTool_{nullptr};

    pp::ControlsTheme theme_;

public:
    Canvas(Rect2f frame, Widget *p, State *s)
        : Widget(frame, p, s)
    {
        theme_.shapeColor   = dr4::Color(255, 255, 255, 255);
        theme_.lineColor    = dr4::Color(255, 0,   0,   255);
        theme_.textColor    = dr4::Color(255, 255, 255, 255);
        theme_.baseFontSize = 16.0f;
        theme_.handleColor  = dr4::Color(255, 0, 0, 255);
    }

    const char *title() const override {
        return "Canvas";
    }

    // ---------- pp::Canvas ----------
    pp::ControlsTheme GetControlsTheme() const override {
        return theme_;
    }

    void AddShape(pp::Shape *shape) override {
        shapes_.emplace_back(shape);
        requestRedraw();
    }

    void DelShape(pp::Shape *shape) override {
        if (selected_ == shape)
            selected_ = nullptr;

        auto it = std::remove_if(shapes_.begin(), shapes_.end(),
                                 [shape](const std::unique_ptr<pp::Shape> &p) {
                                     return p.get() == shape;
                                 });
        shapes_.erase(it, shapes_.end());
        requestRedraw();
    }

    void SetSelectedShape(pp::Shape *shape) override {
        if (selected_ && selected_ != shape)
            selected_->OnDeselect();
        selected_ = shape;
        if (selected_)
            selected_->OnSelect();
        requestRedraw();
    }

    pp::Shape *GetSelectedShape() const override {
        return selected_;
    }

    void ShapeChanged(pp::Shape */*shape*/) override {
        requestRedraw();
    }

    dr4::Window *GetWindow() override {
        return state->window;
    }

    void setTools(std::vector<std::unique_ptr<pp::Tool>> &&tools) {
        // End old active tool
        if (activeTool_)
            activeTool_->OnEnd();

        tools_ = std::move(tools);
        activeTool_ = tools_.empty() ? nullptr : tools_.front().get();

        if (activeTool_)
            activeTool_->OnStart();
    }

    const std::vector<std::unique_ptr<pp::Tool>> &tools() const { return tools_; }
    pp::Tool *activeTool() const { return activeTool_; }
    void setActiveTool(size_t index) {
        if (index >= tools_.size()) return;
        if (activeTool_)
            activeTool_->OnEnd();
        activeTool_ = tools_[index].get();
        if (activeTool_)
            activeTool_->OnStart();
    }

    void draw() override {
        texture->Clear({0, 0, 0, 50});
        for (auto &s : shapes_) {
            s->DrawOn(*texture);
        }
    }

    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *) override {
        if (state->mouse.target != this) return PROPAGATE;
        if (!activeTool_) return PROPAGATE;

        dr4::Event::MouseButton mev{};
        mev.pos    = ctx.mouse_rel;
        mev.button = dr4::MouseButtonType::LEFT;

        if (activeTool_->OnMouseDown(mev))
            return CONSUME;

        return PROPAGATE;
    }

    DispatchResult onMouseUp(DispatcherCtx ctx, const MouseUpEvent *) override {
        if (state->mouse.target != this) return PROPAGATE;
        if (!activeTool_) return PROPAGATE;

        dr4::Event::MouseButton mev{};
        mev.pos    = ctx.mouse_rel;
        mev.button = dr4::MouseButtonType::LEFT;

        if (activeTool_->OnMouseUp(mev))
            return CONSUME;

        return PROPAGATE;
    }

    DispatchResult onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) override {
        dr4::Event::MouseMove mev{};
        mev.pos = ctx.mouse_rel;

        if (activeTool_) {
            requestRedraw();
            if (activeTool_->OnMouseMove(mev)) {
                Widget::onMouseMove(ctx, e);
                return CONSUME;
            }
        }

        return Widget::onMouseMove(ctx, e);
    }
};
