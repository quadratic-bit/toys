#pragma once
#include "./canvas.hpp"
#include "./rgb_picker.hpp"

class CanvasThemePicker final : public RGBPicker {
    Canvas *canvas_;

public:
    CanvasThemePicker(Canvas *c, Rect2f f, Widget *p, State *s)
        : RGBPicker(f, p, s)
        , canvas_(c)
    {
        if (canvas_) {
            auto th = canvas_->GetControlsTheme();
            setColor(th.shapeFillColor);
        }
    }

protected:
    void onColorChanged(dr4::Color c) override {
        if (canvas_) {
            canvas_->SetShapeFillColor(c);
        }
    }
};
