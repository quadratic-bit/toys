#pragma once
#include <functional>

#include <swuix/common.hpp>
#include <swuix/widgets/button.hpp>
#include <swuix/widgets/titled.hpp>
#include <swuix/widgets/textinput.hpp>
#include <swuix/traits/scrollable.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/window/common.hpp>

#include "../trace/objects.hpp"
#include "../materials/material.hpp"

class PropertyTextInput final : public TextInput {
    std::function<void()> on_change;
public:
    PropertyTextInput(Rect2f f, Widget *p, State *s, std::function<void()> cb)
        : Widget(f, p, s), FocusableWidget(f, p, s), TextInput(f, p, s), on_change(std::move(cb)) {}

    const char *title() const override {
        return "Property Textinput";
    }

    void onValueChange() override {
        if (on_change) on_change();
    }

    void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);

        const float inset = 5.0f;
        const float xText = inset - scrollX(); // scroll left when text too wide
        const float yMid  = f.size.y / 2.0f;

        // selection highlight (only when focused)
        if (state->getFocus() == this && selectionActive()) {
            auto [a, b] = selectionRange();
            float x0 = xText + prefixWidthPx(a);
            float x1 = xText + prefixWidthPx(b);
            if (x1 < x0) std::swap(x0, x1);

            r->SetPos({x0, 4.0f});
            r->SetSize({x1 - x0, f.size.y - 8.0f});
            r->SetFillColor(dr4::Color(CLR_PRIMARY, 90));
            r->SetBorderColor(TRANSPARENT);
            texture->Draw(*r);
        }

        // text (left aligned)
        Text *t = textAligned(
            state->window,
            getText().c_str(),
            {xText, yMid},
            Color(CLR_TEXT_STRONG),
            state->appfont,
            16,
            HAlign::LEFT
        );
        texture->Draw(*t);

        // caret
        if (state->getFocus() == this && caretVisible()) {
            float cx = xText + caretXpx();
            dr4::Line *l = thickLine(
                state->window,
                {cx, 4.0f},
                {cx, f.size.y - 4.0f},
                {CLR_TEXT_STRONG},
                1
            );
            texture->Draw(*l);
        }
    }
};

class ObjectViewPropertyEditor final : public Widget {
    Field *property;

    TextInput *input  = nullptr;
    Button    *incBtn = nullptr;
    Button    *decBtn = nullptr;

    float desiredInputW_ = -1.0f;
    bool invalid = false;

    bool isNumeric() const {
        return property->type() == TypeIndex::Double;
    }

    void commitFromInput() {
        if (!input) return;

        const std::string s = input->getText();

        if (s.empty()) {
            invalid = true;
            requestRedraw();
            return;
        }

        try {
            property->deserialize(s);
            invalid = false;
        } catch (...) {
            invalid = true;
        }

        requestRedraw();
        if (parent) parent->requestLayout();
    }

    void step(double delta) {
        if (!isNumeric()) return;

        try {
            double &v = property->as<double>();
            v += delta;
            invalid = false;

            input->setText(property->serialize());
        } catch (...) {
            invalid = true;
        }

        requestRedraw();
        if (parent) parent->requestLayout();
    }

public:
    ObjectViewPropertyEditor(Field *prop, Rect2f f, Widget *p, State *s)
        : Widget(f, p, s), property(prop)
    {
        const float pad = 4.0f;
        const float rowH = f.size.y;

        const float buttonsW = isNumeric() ? (rowH * 0.6f) : 0.0f;
        const float inputRightReserve = isNumeric() ? (buttonsW + pad) : 0.0f;

        const float labelW = f.size.x * 0.45f;

        Rect2f inputFrame {
            labelW,
            pad,
            f.size.x - labelW - inputRightReserve - pad,
            rowH - pad * 2.0f
        };

        input = new PropertyTextInput(
            inputFrame,
            nullptr,
            s,
            [this]() { this->commitFromInput(); }
        );
        input->setText(property->serialize());
        appendChild(input);

        if (isNumeric()) {
            Rect2f incFrame {
                f.size.x - buttonsW,
                pad,
                buttonsW,
                (rowH - pad * 2.0f) * 0.5f
            };
            Rect2f decFrame {
                f.size.x - buttonsW,
                pad + (rowH - pad * 2.0f) * 0.5f,
                buttonsW,
                (rowH - pad * 2.0f) * 0.5f
            };

            incBtn = new Button(incFrame, this, "^", s,
                [](void *st, Widget *w) {
                    (void)st;
                    auto *self = static_cast<ObjectViewPropertyEditor*>(w->parent);
                    self->input->requestRedraw();
                    self->step(+1.0);
                }
            );

            decBtn = new Button(decFrame, this, "v", s,
                [](void *st, Widget *w) {
                    (void)st;
                    auto *self = static_cast<ObjectViewPropertyEditor*>(w->parent);
                    self->step(-1.0);
                    self->input->requestRedraw();
                }
            );

            appendChild(incBtn);
            appendChild(decBtn);
        }
    }

    const char *title() const override {
        return "ObjectView property editor";
    }

    void setDesiredInputWidth(float w) {
        desiredInputW_ = w;
        requestLayout();
        requestRedraw();
    }

    float inputTextWidthPx() const {
        return input ? input->textWidthPx() : 0.0f;
    }

    void layout() override {
        const float pad = 4.0f;
        const float rowH = frame().size.y;

        const float buttonsW = isNumeric() ? (rowH * 0.6f) : 0.0f;
        const float rightReserve = isNumeric() ? (buttonsW + pad) : 0.0f;

        float inputW = desiredInputW_ > 0.0f
            ? desiredInputW_
            : std::max(60.0f, frame().size.x * 0.45f);

        if (isNumeric()) {
            inputW = std::max(60.0f, inputW - rightReserve);
        }

        // clamp inputW to what fits
        float maxInputW = frame().size.x - rightReserve - pad - 60.0f; // leave at least 60 for label
        inputW = std::clamp(inputW, 60.0f, std::max(60.0f, maxInputW));

        float labelW = frame().size.x - inputW - rightReserve - pad;
        labelW = std::max(60.0f, labelW);

        Rect2f inputFrame {
            labelW,
            pad,
            inputW,
            rowH - pad * 2.0f
        };

        if (input) {
            input->position = inputFrame.pos;
            input->texture->SetSize(inputFrame.size);
            input->requestLayout();
            input->requestRedraw();
        }

        if (isNumeric() && incBtn && decBtn) {
            Rect2f incFrame {
                frame().size.x - buttonsW,
                pad,
                buttonsW,
                (rowH - pad * 2.0f) * 0.5f
            };
            Rect2f decFrame {
                frame().size.x - buttonsW,
                pad + (rowH - pad * 2.0f) * 0.5f,
                buttonsW,
                (rowH - pad * 2.0f) * 0.5f
            };

            incBtn->position = incFrame.pos;
            incBtn->texture->SetSize(incFrame.size);
            incBtn->requestRedraw();

            decBtn->position = decFrame.pos;
            decBtn->texture->SetSize(decFrame.size);
            decBtn->requestRedraw();
        }
    }

    void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectFill(state->window, f, {CLR_SURFACE_2});
        texture->Draw(*r);

        if (invalid) {
            r = outline(state->window, frame(), 1, dr4::Color(255, 0, 0));
        } else {
            r = outline(state->window, frame(), 1, dr4::Color(CLR_BORDER));
        }
        texture->Draw(*r);

        Text *t = textAligned(
            state->window,
            property->name().c_str(),
            {5, texture->GetHeight() / 2},
            Color(CLR_TEXT_STRONG),
            state->appfont
        );
        texture->Draw(*t);
    }
};

class ObjectViewPropertyList final : public ScrollableWidget {
    Object *obj;
    FieldList fields;

    VScrollbar *scrollbar_ = nullptr;

public:
    ObjectViewPropertyList(Object *o, Rect2f f, Vec2f clip, Widget *p, State *s)
        : Widget(f, p, s)
        , ScrollableWidget(f, clip, p, s)
        , obj(o)
    {
        obj->collectFields(fields);
        obj->mat->collectFields(fields);

        float y = 0.0f;
        for (size_t i = 0; i < fields.size(); ++i) {
            auto *property = new ObjectViewPropertyEditor(fields[i],
                {0, y, f.size.x, 35}, nullptr, s);
            appendChild(property);
            y += 35.0f;
        }

        texture->SetSize({texture->GetWidth(), y + 10.0f});

        scrollbar_ = new VScrollbar(state);
        scrollbar_->attachTo(this);

        requestLayout();
        requestRedraw();
    }

    const char *title() const override {
        return "ObjectView property list";
    }

    void layout() override {
        const float viewportW = viewport->GetWidth();
        const float viewportH = viewport->GetHeight();

        const float rowH = 35.0f;
        const float contentW = std::max(40.0f, viewportW - SCROLLBAR_W);

        float maxPx = 0.0f;
        for (Widget *c : children) {
            if (auto *ed = dynamic_cast<ObjectViewPropertyEditor*>(c))
                maxPx = std::max(maxPx, ed->inputTextWidthPx());
        }

        // pixels -> desired input box width (padding + a little slack)
        float desired = maxPx + 16.0f;

        // clamp to what fits in the panel
        float maxFit = std::max(60.0f, viewportW - SCROLLBAR_W - 80.0f); // 80px min label
        desired = std::clamp(desired, 80.0f, maxFit);

        for (Widget *c : children) {
            if (auto *ed = dynamic_cast<ObjectViewPropertyEditor*>(c))
                ed->setDesiredInputWidth(desired);
        }

        float y = 0.0f;
        for (Widget *c : children) {
            auto *ed = dynamic_cast<ObjectViewPropertyEditor*>(c);
            if (!ed) continue; // skip scrollbar + buttons

            ed->position = {0.0f, y};
            ed->texture->SetSize({contentW, rowH});
            ed->requestLayout();
            y += rowH;
        }

        float contentH = y + 10.0f;
        texture->SetSize({viewportW, std::max(contentH, viewportH)});

        if (scrollbar_) {
            float progress_px = contentProgressY();

            scrollbar_->position.x = texture->GetWidth() - SCROLLBAR_W;
            scrollbar_->position.y = progress_px;
            scrollbar_->texture->SetSize({SCROLLBAR_W, viewportH});

            float trackH = scrollbar_->scrollHeight();
            float ratio = (texture->GetHeight() <= 1.0f) ? 1.0f : (viewportH / texture->GetHeight());
            ratio = std::clamp(ratio, 0.05f, 1.0f);

            float sliderH = std::max(10.0f, trackH * ratio);
            scrollbar_->slider->texture->SetSize({scrollbar_->slider->texture->GetWidth(), sliderH});
            scrollbar_->slider->position.y = SCROLL_BUT_H + scrollbar_->scrollProgress();

            scrollbar_->layout();
            scrollbar_->slider->layout();
            scrollbar_->slider->requestRedraw();
        }

        requestRedraw();
    }

    void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectFill(state->window, f, {CLR_SURFACE_2});
        texture->Draw(*r);
    }
};

class ObjectPreview;

class ObjectView final : public TitledWidget {
public:
    ObjectView(Rect2f f, Widget *p, State *s)
            : Widget(f, p, s), TitledWidget(f, p, s) {}

	const char *title() const override {
		return "Properties";
	}

    void unpopulate() {
        while (children.size() > 1) {
            delete children[children.size() - 1];
            children.pop_back();
        }
        requestRedraw();
    }

    void populate(ObjectPreview *prev);

	void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_1}, 2, {CLR_BORDER});
        texture->Draw(*r);
	}
};
