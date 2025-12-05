#pragma once
#include <cstdio>
#include <memory>

#include "dr4/keycodes.hpp"
#include "extern/window.hpp"
#include "swuix/traits/focusable.hpp"
#include "swuix/widget.hpp"
#include "swuix/state.hpp"

#include "pp/canvas.hpp"
#include "pp/tool.hpp"

class Canvas final : public pp::Canvas, public FocusableWidget {
    // owned by this canvas
    std::vector<std::unique_ptr<pp::Shape>> shapes_;
    pp::Shape *selected_{nullptr};

    // tools
    std::vector<std::unique_ptr<pp::Tool>> tools_;
    pp::Tool *activeTool_{nullptr};

    pp::ControlsTheme theme_;

public:
    Canvas(Rect2f frame, Widget *p, State *s)
        : Widget(frame, p, s), FocusableWidget(frame, p, s)
    {
        theme_.shapeBorderColor  = dr4::Color(255, 255, 255, 255);
        theme_.shapeFillColor    = dr4::Color(255, 0,   0,   255);
        theme_.selectColor       = dr4::Color(0,   255, 0,   255);
        theme_.textColor         = dr4::Color(255, 255, 255, 255);
        theme_.baseFontSize      = 16.0f;
        theme_.handleColor       = dr4::Color(255, 0, 0, 255);
        theme_.handleHoverColor  = dr4::Color(255, 0, 0, 255);
        theme_.handleActiveColor = dr4::Color(255, 0, 0, 255);
    }

    dr4::Texture *getTexture() const { return texture; }

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
        if (activeTool_)
            activeTool_->OnEnd();

        tools_ = std::move(tools);
        activeTool_ = nullptr;
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
        state->focus(this);

        dr4::Event::MouseButton mev{};
        mev.pos    = ctx.mouse_rel;
        mev.button = dr4::MouseButtonType::LEFT;

        for (auto it = shapes_.rbegin(); it != shapes_.rend(); ++it) {
            pp::Shape *shape = it->get();
            if (shape->OnMouseDown(mev)) {
                SetSelectedShape(shape);
                return CONSUME;
            }
        }

        if (!activeTool_) return PROPAGATE;

        if (activeTool_->OnMouseDown(mev))
            return CONSUME;

        return PROPAGATE;
    }

    DispatchResult onMouseUp(DispatcherCtx ctx, const MouseUpEvent *) override {
        if (state->mouse.target != this) return PROPAGATE;

        dr4::Event::MouseButton mev{};
        mev.pos    = ctx.mouse_rel;
        mev.button = dr4::MouseButtonType::LEFT;

        if (activeTool_) {
            requestRedraw();
            if (activeTool_->OnMouseUp(mev))
                return CONSUME;
        }

        if (selected_) {
            requestRedraw();
            if (selected_->OnMouseUp(mev))
                return CONSUME;
        }

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

        bool any = false;
        for (auto &s : shapes_) {
            if (s->OnMouseMove(mev))
                any = true;
        }

        if (any) {
            requestRedraw();
            Widget::onMouseMove(ctx, e);
            return CONSUME;
        }

        return Widget::onMouseMove(ctx, e);
    }

    DispatchResult onKeyDown(DispatcherCtx ctx, const KeyDownEvent *e) override {
        if (state->getFocus() != this) return PROPAGATE;
        dr4::Event::KeyEvent kev{};
        kev.sym = (dr4::KeyCode)e->keycode;
        kev.mods = e->mods;

        if ((e->mods & dr4::KEYMOD_CTRL) && e->keycode == dr4::KEYCODE_P) {
            parent->onKeyDown(ctx, e);
        }

        if (activeTool_) {
            requestRedraw();
            if (activeTool_->OnKeyDown(kev)) {
                return CONSUME;
            }
        }

        return PROPAGATE;
    }

    DispatchResult onKeyUp(DispatcherCtx, const KeyUpEvent *e) override {
        if (state->getFocus() != this) return PROPAGATE;
        dr4::Event::KeyEvent kev{};
        kev.sym = MapSDLKey(e->keycode);
        kev.mods = e->mods;

        if (activeTool_) {
            requestRedraw();
            if (activeTool_->OnKeyUp(kev)) {
                return CONSUME;
            }
        }

        return PROPAGATE;
    }

    DispatchResult onInput(DispatcherCtx, const InputEvent *e) override {
        if (state->getFocus() != this) return PROPAGATE;
        dr4::Event::TextEvent tev{};
        tev.unicode = e->text;

        if (activeTool_) {
            requestRedraw();
            if (activeTool_->OnText(tev)) {
                return CONSUME;
            }
        }

        return PROPAGATE;
    }

    DispatchResult onIdle(DispatcherCtx, const IdleEvent *e) override {
        pp::IdleEvent idle{state->window->GetTime(), e->dt_s};

        if (activeTool_) {
            activeTool_->OnIdle(idle);
        }

        for (auto &s : shapes_) {
            s->OnIdle(idle);
        }
        return PROPAGATE;
    }
};
