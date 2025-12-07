#ifndef RECT_TOOL
#define RECT_TOOL

#include <memory>
#include <array>
#include <algorithm>

#include <pp/shape.hpp>
#include <pp/canvas.hpp>
#include <pp/tool.hpp>

#include <dr4/math/vec2.hpp>
#include <dr4/math/color.hpp>
#include <dr4/math/rect.hpp>

#include <cum/ifc/pp.hpp>
#include <cum/plugin.hpp>

namespace {

using dr4::Vec2f;
using dr4::Color;

class RectShape final : public pp::Shape {
    pp::Canvas *canvas_{nullptr};

    Vec2f pos_{0.f, 0.f};   // top-left
    Vec2f size_{0.f, 0.f};  // width/height (kept non-negative by normalize())

    Color border_{255, 255, 255, 255};
    Color fill_{0, 0, 0, 0};
    float borderThickness_{2.0f};

    bool selected_{false};

    enum class Handle {
        NONE = 0,
        TL, TM, TR,
        ML,     MR,
        BL, BM, BR
    };

    enum class DragMode { NONE, MOVE, RESIZE };

    DragMode drag_{DragMode::NONE};
    Handle   active_{Handle::NONE};
    Handle   hovered_{Handle::NONE};

    Vec2f dragOffset_{0.f, 0.f};

    Vec2f startPos_{0.f, 0.f};
    Vec2f startSize_{0.f, 0.f};

    static constexpr float HandleSize = 8.0f;

    static Color WithAlpha(Color c, uint8_t a) {
        c.a = a;
        return c;
    }

    static float absf(float v) { return v < 0.f ? -v : v; }

    void normalize() {
        // ensure size is non-negative while preserving the same rectangle area
        if (size_.x < 0.f) {
            pos_.x += size_.x;
            size_.x = -size_.x;
        }
        if (size_.y < 0.f) {
            pos_.y += size_.y;
            size_.y = -size_.y;
        }
    }

    dr4::Rect2f rect() const {
        return dr4::Rect2f(pos_.x, pos_.y, size_.x, size_.y);
    }

    std::array<std::pair<Handle, Vec2f>, 8> handlePoints() const {
        const float x0 = pos_.x;
        const float y0 = pos_.y;
        const float x1 = pos_.x + size_.x;
        const float y1 = pos_.y + size_.y;
        const float xm = (x0 + x1) * 0.5f;
        const float ym = (y0 + y1) * 0.5f;

        return {{
            {Handle::TL, {x0, y0}},
            {Handle::TM, {xm, y0}},
            {Handle::TR, {x1, y0}},
            {Handle::ML, {x0, ym}},
            {Handle::MR, {x1, ym}},
            {Handle::BL, {x0, y1}},
            {Handle::BM, {xm, y1}},
            {Handle::BR, {x1, y1}},
        }};
    }

    dr4::Rect2f handleRect(Vec2f p) const {
        return dr4::Rect2f(
            p.x - HandleSize * 0.5f,
            p.y - HandleSize * 0.5f,
            HandleSize,
            HandleSize
        );
    }

    Handle hitHandle(Vec2f p) const {
        for (auto &hp : handlePoints()) {
            if (handleRect(hp.second).Contains(p))
                return hp.first;
        }
        return Handle::NONE;
    }

    void beginResize(Handle h) {
        active_ = h;
        drag_   = DragMode::RESIZE;
        startPos_  = pos_;
        startSize_ = size_;
    }

    void updateResize(Vec2f mouse) {
        // Work with edges from the starting rect
        float x0 = startPos_.x;
        float y0 = startPos_.y;
        float x1 = startPos_.x + startSize_.x;
        float y1 = startPos_.y + startSize_.y;

        switch (active_) {
            case Handle::TL: x0 = mouse.x; y0 = mouse.y; break;
            case Handle::TR: x1 = mouse.x; y0 = mouse.y; break;
            case Handle::BL: x0 = mouse.x; y1 = mouse.y; break;
            case Handle::BR: x1 = mouse.x; y1 = mouse.y; break;

            case Handle::TM: y0 = mouse.y; break;
            case Handle::BM: y1 = mouse.y; break;
            case Handle::ML: x0 = mouse.x; break;
            case Handle::MR: x1 = mouse.x; break;

            default: break;
        }

        pos_  = { std::min(x0, x1), std::min(y0, y1) };
        size_ = { absf(x1 - x0), absf(y1 - y0) };

        if (canvas_) canvas_->ShapeChanged(this);
    }

public:
    RectShape(Vec2f pos, Vec2f size, Color border, Color fill,
              float thickness, pp::Canvas *c)
        : canvas_(c)
        , pos_(pos)
        , size_(size)
        , border_(border)
        , fill_(fill)
        , borderThickness_(thickness) {
        normalize();
    }

    void SetFromPoints(Vec2f a, Vec2f b) {
        pos_  = { std::min(a.x, b.x), std::min(a.y, b.y) };
        size_ = { absf(b.x - a.x), absf(b.y - a.y) };
        if (canvas_) canvas_->ShapeChanged(this);
    }

    void SetBorderColor(Color c) { border_ = c; if (canvas_) canvas_->ShapeChanged(this); }
    void SetFillColor(Color c)   { fill_ = c;   if (canvas_) canvas_->ShapeChanged(this); }
    void SetBorderThickness(float t) { borderThickness_ = t; if (canvas_) canvas_->ShapeChanged(this); }

    void SetPos(Vec2f pos) override {
        pos_ = pos;
        normalize();
        if (canvas_) canvas_->ShapeChanged(this);
    }

    Vec2f GetPos() const override {
        return pos_;
    }

    bool OnMouseDown(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;
        if (evt.button != MBT::LEFT) return false;

        // Prefer handles when selected
        if (selected_) {
            Handle h = hitHandle(evt.pos);
            if (h != Handle::NONE) {
                beginResize(h);
                return true;
            }
        }

        // Allow moving by dragging inside the rect
        if (rect().Contains(evt.pos)) {
            drag_ = DragMode::MOVE;
            dragOffset_ = evt.pos - pos_;
            return true;
        }

        return false;
    }

    bool OnMouseMove(const dr4::Event::MouseMove &evt) override {
        if (drag_ == DragMode::MOVE) {
            pos_ = evt.pos - dragOffset_;
            normalize();
            if (canvas_) canvas_->ShapeChanged(this);
            return true;
        }

        if (drag_ == DragMode::RESIZE) {
            updateResize(evt.pos);
            return true;
        }

        if (selected_) {
            Handle h = hitHandle(evt.pos);
            if (h != hovered_) {
                hovered_ = h;
                if (canvas_) canvas_->ShapeChanged(this);
            }
            return hovered_ != Handle::NONE;
        }

        return false;
    }

    bool OnMouseUp(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;
        if (evt.button != MBT::LEFT) return false;

        if (drag_ != DragMode::NONE) {
            drag_ = DragMode::NONE;
            active_ = Handle::NONE;
            hovered_ = Handle::NONE;
            if (canvas_) canvas_->ShapeChanged(this);
            return true;
        }
        return false;
    }

    void OnSelect() override {
        selected_ = true;
        if (canvas_) canvas_->ShapeChanged(this);
    }

    void OnDeselect() override {
        selected_ = false;
        drag_ = DragMode::NONE;
        active_ = Handle::NONE;
        hovered_ = Handle::NONE;
        if (canvas_) canvas_->ShapeChanged(this);
    }

    void DrawOn(dr4::Texture &tex) const override {
        auto *wnd = canvas_ ? canvas_->GetWindow() : nullptr;
        if (!wnd) return;

        // Main rectangle
        std::unique_ptr<dr4::Rectangle> r(wnd->CreateRectangle());
        if (r) {
            r->SetPos(pos_);
            r->SetSize(size_);
            r->SetFillColor(fill_);
            r->SetBorderThickness(borderThickness_);
            r->SetBorderColor(border_);
            r->DrawOn(tex);
        }

        if (!selected_) return;

        auto theme = canvas_->GetControlsTheme();

        auto drawHandle = [&](Vec2f p, bool hovered) {
            std::unique_ptr<dr4::Rectangle> hr(wnd->CreateRectangle());
            if (!hr) return;

            auto rc = handleRect(p);

            hr->SetPos(rc.pos);
            hr->SetSize(rc.size);

            const uint8_t a = hovered ? 255 : 120;

            hr->SetFillColor(WithAlpha(theme.handleColor, a));
            hr->SetBorderThickness(1.0f);
            hr->SetBorderColor(WithAlpha(theme.selectColor, a));

            hr->DrawOn(tex);
        };

        for (auto &hp : handlePoints()) {
            drawHandle(hp.second, hovered_ == hp.first);
        }
    }
};

class RectTool final : public pp::Tool {
    pp::Canvas *canvas_{nullptr};
    RectShape  *current_{nullptr};
    Vec2f       startPos_{0.f, 0.f};
    bool        drawing_{false};

    static Vec2f mousePos(const dr4::Event::MouseButton &e) { return e.pos; }
    static Vec2f mousePos(const dr4::Event::MouseMove   &e) { return e.pos; }

public:
    explicit RectTool(pp::Canvas *cvs) : canvas_(cvs) {}

    std::string_view Icon() const override {
        static constexpr std::string_view icon = u8"󰹞";
        return icon;
    }

    std::string_view Name() const override {
        static constexpr std::string_view name = "Rectangle";
        return name;
    }

    bool IsCurrentlyDrawing() const override {
        return drawing_;
    }

    void OnStart() override {}

    void OnBreak() override {
        if (drawing_ && current_) {
            canvas_->DelShape(current_);
            current_ = nullptr;
            drawing_ = false;
        }
    }

    void OnEnd() override {
        drawing_ = false;
        current_ = nullptr;
    }

    bool OnMouseDown(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;

        if (evt.button != MBT::LEFT) return false;
        if (drawing_) return false;

        startPos_ = mousePos(evt);

        auto theme = canvas_->GetControlsTheme();

        auto *shape = new RectShape(startPos_, {0.f, 0.f},
                                    theme.shapeBorderColor,
                                    theme.shapeFillColor,
                                    2.0f,
                                    canvas_);

        canvas_->AddShape(shape);

        current_ = shape;
        drawing_ = true;
        return true;
    }

    bool OnMouseMove(const dr4::Event::MouseMove &evt) override {
        if (!drawing_ || !current_) return false;

        Vec2f p = mousePos(evt);
        current_->SetFromPoints(startPos_, p);
        return true;
    }

    bool OnMouseUp(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;

        if (!drawing_ || !current_) return false;
        if (evt.button != MBT::LEFT) return false;

        Vec2f p = mousePos(evt);
        current_->SetFromPoints(startPos_, p);

        drawing_ = false;
        current_ = nullptr;
        return true;
    }
};

} // namespace

#endif
