#include <memory>

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

class LineShape final : public pp::Shape {
    Vec2f start_{0.f, 0.f};
    Vec2f end_  {0.f, 0.f};
    Color color_{255, 255, 255, 255};
    float thickness_{1.0f};
    pp::Canvas *canvas_;

    bool hoverStart_{false};
    bool hoverEnd_{false};

    enum class DragMode { NONE, START, END };
    DragMode drag_{DragMode::NONE};

    static constexpr float HandleSize = 8.0f;

    static Color WithAlpha(Color c, uint8_t a) {
        c.a = a;
        return c;
    }

    dr4::Rect2f HandleRect(Vec2f p) const {
        return dr4::Rect2f(
            p.x - HandleSize * 0.5f,
            p.y - HandleSize * 0.5f,
            HandleSize,
            HandleSize
        );
    }

    static float dist2(Vec2f a, Vec2f b) {
        Vec2f d = a - b;
        return d.x * d.x + d.y * d.y;
    }

public:
    LineShape(const Vec2f &a,
              const Vec2f &b,
              Color color,
              float thickness,
              pp::Canvas *c)
        : start_(a)
        , end_(b)
        , color_(color)
        , thickness_(thickness)
        , canvas_(c) {}

    void SetEndpoints(const Vec2f &a, const Vec2f &b) {
        start_ = a;
        end_   = b;
    }

    bool OnMouseDown(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;
        if (evt.button != MBT::LEFT) return false;

        if (HandleRect(start_).Contains(evt.pos)) {
            drag_ = DragMode::START;
            return true;
        }
        if (HandleRect(end_).Contains(evt.pos)) {
            drag_ = DragMode::END;
            return true;
        }

        return false;
    }

    bool OnMouseMove(const dr4::Event::MouseMove &evt) override {
        if (drag_ == DragMode::START) {
            start_ = evt.pos;
            return true;
        }
        if (drag_ == DragMode::END) {
            end_ = evt.pos;
            return true;
        }

        bool hs = HandleRect(start_).Contains(evt.pos);
        bool he = HandleRect(end_).Contains(evt.pos);

        if (hs != hoverStart_ || he != hoverEnd_) {
            hoverStart_ = hs;
            hoverEnd_   = he;
            canvas_->ShapeChanged(this);
        }

        return hoverStart_ || hoverEnd_;
    }

    bool OnMouseUp(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;
        if (evt.button != MBT::LEFT) return false;

        if (drag_ != DragMode::NONE) {
            drag_ = DragMode::NONE;
            hoverStart_ = false;
            hoverEnd_ = false;
            return true;
        }
        return false;
    }

    void SetPos(Vec2f pos) override {
        Vec2f delta = pos - start_;
        start_ += delta;
        end_   += delta;
    }

    Vec2f GetPos() const override {
        return start_;
    }

    void DrawOn(dr4::Texture &tex) const override {
        std::unique_ptr<dr4::Line> ln(canvas_->GetWindow()->CreateLine());
        ln->SetStart(start_);
        ln->SetEnd(end_);
        ln->SetColor(color_);
        ln->SetThickness(thickness_);
        ln->DrawOn(tex);

        auto theme = canvas_->GetControlsTheme();

        auto drawHandle = [&](Vec2f p, bool hovered) {
            std::unique_ptr<dr4::Rectangle> r(canvas_->GetWindow()->CreateRectangle());
            if (!r) return;

            auto rect = HandleRect(p);

            r->SetPos(rect.pos);
            r->SetSize(rect.size);

            const uint8_t a = hovered ? 255 : 110;

            r->SetFillColor(WithAlpha(theme.handleColor, a));
            r->SetBorderThickness(1.0f);
            r->SetBorderColor(WithAlpha(theme.selectColor, a));

            r->DrawOn(tex);
        };

        drawHandle(start_, hoverStart_);
        drawHandle(end_,   hoverEnd_);
    }
};

class LineTool : public pp::Tool {
    pp::Canvas *canvas_{nullptr};
    LineShape  *current_{nullptr};
    Vec2f       startPos_{0.f, 0.f};
    bool        drawing_{false};

    static Vec2f mousePos(const dr4::Event::MouseButton &e) {
        return e.pos;
    }

    static Vec2f mousePos(const dr4::Event::MouseMove &e) {
        return e.pos;
    }

public:
    explicit LineTool(pp::Canvas *cvs) : canvas_(cvs) {}

    std::string_view Icon() const override {
        static constexpr std::string_view icon = u8"󰕞";
        return icon;
    }

    std::string_view Name() const override {
        static constexpr std::string_view name = "Line";
        return name;
    }

    bool IsCurrentlyDrawing() const override {
        return drawing_;
    }

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

        auto *shape = new LineShape(startPos_, startPos_,
                                    theme.shapeBorderColor,
                                    3.0f, canvas_);
        canvas_->AddShape(shape);

        current_ = shape;
        drawing_ = true;
        return true;
    }

    bool OnMouseMove(const dr4::Event::MouseMove &evt) override {
        if (!drawing_ || !current_)
            return false;

        Vec2f p = mousePos(evt);
        current_->SetEndpoints(startPos_, p);
        canvas_->ShapeChanged(current_);
        return true;
    }

    bool OnMouseUp(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;

        if (!drawing_ || !current_) return false;
        if (evt.button != MBT::LEFT) return false;

        Vec2f p = mousePos(evt);
        current_->SetEndpoints(startPos_, p);
        canvas_->ShapeChanged(current_);

        drawing_ = false;
        current_ = nullptr;
        return true;
    }
};

}
