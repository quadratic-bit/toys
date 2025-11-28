#include <memory>

#include "pp/shape.hpp"
#include "pp/canvas.hpp"
#include "pp/tool.hpp"

#include "dr4/math/vec2.hpp"
#include "dr4/math/color.hpp"

#include "cum/ifc/pp.hpp"
#include "cum/plugin.hpp"

namespace {

using dr4::Vec2f;
using dr4::Color;

class LineShape final : public pp::Shape {
    Vec2f start_{0.f, 0.f};
    Vec2f end_  {0.f, 0.f};
    Color color_{255, 255, 255, 255};
    float thickness_{1.0f};
    pp::Canvas *canvas_;

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

    // ---- dr4::Drawable ----
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
    }
};

class LineTool : public pp::Tool {
    pp::Canvas *canvas_{nullptr};
    LineShape  *current_{nullptr};  // owned by canvas
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

    // ---- metadata ----
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

    // ---- lifecycle ----
    void OnBreak() override {
        // ESC while drawing => cancel the line
        if (drawing_ && current_) {
            canvas_->DelShape(current_);
            // Canvas is responsible for deleting the shape.
            current_ = nullptr;
            drawing_ = false;
        }
    }

    void OnEnd() override {
        // Tool deselected; stop drawing but keep the last line
        drawing_ = false;
        current_ = nullptr;
    }

    // ---- events ----
    bool OnMouseDown(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;

        if (evt.button != MBT::LEFT) return false;
        if (drawing_) return false;

        startPos_ = mousePos(evt);
        auto theme = canvas_->GetControlsTheme();

        auto *shape = new LineShape(startPos_, startPos_,
                                    theme.lineColor,
                                    3.0f, canvas_);
        canvas_->AddShape(shape);
        canvas_->SetSelectedShape(shape);

        current_ = shape;
        drawing_ = true;
        return true;   // event consumed
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

class LineToolPlugin : public cum::PPToolPlugin {
public:
    // ----- cum::Plugin -----
    std::string_view GetIdentifier() const override {
        return "pp.tools.line";
    }

    std::string_view GetName() const override {
        return "Line drawing tool";
    }

    std::string_view GetDescription() const override {
        return "Simple line-drawing tool for pp::Canvas";
    }

    std::vector<std::string_view> GetDependencies() const override { return {}; }
    std::vector<std::string_view> GetConflicts() const override { return {}; }

    void AfterLoad() override {}

    // ----- cum::PPToolPlugin -----
    std::vector<std::unique_ptr<pp::Tool>> CreateTools(pp::Canvas *cvs) override {
        std::vector<std::unique_ptr<pp::Tool>> tools;
        tools.emplace_back(std::make_unique<LineTool>(cvs));
        return tools;
    }
};

}

extern "C" cum::Plugin *CreatePlugin();
extern "C" cum::Plugin *CreatePlugin() {
    return new LineToolPlugin();
}
