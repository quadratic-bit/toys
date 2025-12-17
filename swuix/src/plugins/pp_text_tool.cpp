#ifndef TEXT_TOOL
#define TEXT_TOOL

#include <memory>
#include <string>
#include <cmath>
#include <algorithm>
#include <cctype>

#include <pp/shape.hpp>
#include <pp/canvas.hpp>
#include <pp/tool.hpp>

#include <dr4/event.hpp>
#include <dr4/math/vec2.hpp>
#include <dr4/math/color.hpp>
#include <dr4/texture.hpp>

#include <cum/ifc/pp.hpp>
#include <cum/plugin.hpp>

#include "../swuix/pp_text_font.inl"
const size_t g_TextToolFontSize = g_TextToolFontData_len;

namespace {

using dr4::Vec2f;
using dr4::Color;

class TextShape final : public pp::Shape {
    pp::Canvas      *canvas_{nullptr};
    Vec2f            pos_{0.f, 0.f};
    std::string      text_;
    Color            color_{255, 255, 255, 255};
    float            fontSize_{16.f};
    const dr4::Font *font_{nullptr};

    bool  dragging_{false};
    Vec2f dragOffset_{0.f, 0.f};

    bool  selected_{false};
    bool  editing_{false};
    bool  caretVisible_{true};

    size_t caret_{0};
    bool   selectingText_{false};
    size_t selAnchor_{0};

    static constexpr float Padding = 3.0f;

    mutable bool  boundsDirty_{true};
    mutable Vec2f cachedBounds_{0.f, 0.f};

    Vec2f measureText() const {
        if (!canvas_) return cachedBounds_;
        if (!boundsDirty_) return cachedBounds_;

        dr4::Window *wnd = canvas_->GetWindow();
        if (!wnd) return cachedBounds_;

        std::unique_ptr<dr4::Text> t(wnd->CreateText());
        if (!t) return cachedBounds_;

        if (font_) t->SetFont(font_);
        t->SetText(text_);
        t->SetFontSize(fontSize_);
        t->SetVAlign(dr4::Text::VAlign::TOP);

        cachedBounds_ = t->GetBounds();
        boundsDirty_  = false;
        return cachedBounds_;
    }

    void markDirty() {
        boundsDirty_ = true;
        if (canvas_) {
            canvas_->ShapeChanged(this);
        }
    }

    static bool isInside(Vec2f p, Vec2f origin, Vec2f size) {
        const float x0 = origin.x;
        const float y0 = origin.y;
        const float x1 = origin.x + size.x;
        const float y1 = origin.y + size.y;
        return (p.x >= x0 && p.x <= x1 && p.y >= y0 && p.y <= y1);
    }

    static bool isCont(uint8_t c) { return (c & 0xC0u) == 0x80u; }

    size_t clampIndex(size_t i) const {
        return std::min(i, text_.size());
    }

    size_t prevCodepoint(size_t i) const {
        if (i == 0) return 0;
        i = std::min(i, text_.size());
        do { --i; } while (i > 0 && isCont(static_cast<uint8_t>(text_[i])));
        return i;
    }

    size_t nextCodepoint(size_t i) const {
        i = std::min(i, text_.size());
        if (i >= text_.size()) return text_.size();
        ++i;
        while (i < text_.size() && isCont(static_cast<uint8_t>(text_[i])))
            ++i;
        return i;
    }

    static bool isAsciiWordChar(unsigned char c) {
        return std::isalnum(c) || c == '_';
    }

    size_t prevWord(size_t i) const {
        i = std::min(i, text_.size());
        if (i == 0) return 0;

        i = prevCodepoint(i);

        while (i > 0 && std::isspace(static_cast<unsigned char>(text_[i]))) {
            i = prevCodepoint(i);
        }

        while (i > 0) {
            size_t pi = prevCodepoint(i);
            if (std::isspace(static_cast<unsigned char>(text_[pi])))
                break;
            i = pi;
        }

        return i;
    }

    size_t nextWord(size_t i) const {
        i = std::min(i, text_.size());

        while (i < text_.size() &&
               std::isspace(static_cast<unsigned char>(text_[i]))) {
            i = nextCodepoint(i);
        }

        while (i < text_.size() &&
               !std::isspace(static_cast<unsigned char>(text_[i]))) {
            i = nextCodepoint(i);
        }

        while (i < text_.size() &&
               std::isspace(static_cast<unsigned char>(text_[i]))) {
            i = nextCodepoint(i);
        }

        return i;
    }

    bool hasSelection() const {
        return selAnchor_ != caret_;
    }

    std::pair<size_t,size_t> selRange() const {
        size_t a = selAnchor_;
        size_t b = caret_;
        if (a > b) std::swap(a, b);
        return {a, b};
    }

    void clearSelection() {
        selAnchor_ = caret_;
    }

    void eraseSelection() {
        if (!hasSelection()) return;
        auto [a, b] = selRange();
        text_.erase(a, b - a);
        caret_ = a;
        selAnchor_ = a;
        markDirty();
    }

    void insertAtCaret(const std::string &utf8) {
        text_.insert(caret_, utf8);
        caret_ += utf8.size();
        selAnchor_ = caret_;
        markDirty();
    }

    size_t indexFromX(float x) const {
        auto *wnd = canvas_ ? canvas_->GetWindow() : nullptr;
        if (!wnd) return 0;

        float localX = x - pos_.x;
        if (localX <= 0.f) return 0;

        std::unique_ptr<dr4::Text> t(wnd->CreateText());
        if (!t) return 0;

        if (font_) t->SetFont(font_);
        t->SetFontSize(fontSize_);
        t->SetVAlign(dr4::Text::VAlign::TOP);

        size_t i = 0;
        size_t last = 0;

        while (i < text_.size()) {
            size_t ni = nextCodepoint(i);
            t->SetText(text_.substr(0, ni));
            float w = t->GetBounds().x;
            if (w >= localX) return ni;
            last = ni;
            i = ni;
        }
        return last;
    }

    float caretOffsetX() const {
        auto *wnd = canvas_ ? canvas_->GetWindow() : nullptr;
        if (!wnd) return 0.f;

        std::unique_ptr<dr4::Text> t(wnd->CreateText());
        if (!t) return 0.f;

        if (font_) t->SetFont(font_);
        t->SetFontSize(fontSize_);
        t->SetVAlign(dr4::Text::VAlign::TOP);

        size_t i = std::min(caret_, text_.size());
        t->SetText(text_.substr(0, i));
        return t->GetBounds().x;
    }

public:
    std::string selectedText() const {
        if (!hasSelection()) return {};
        auto [a, b] = selRange();
        return text_.substr(a, b - a);
    }

    TextShape(pp::Canvas *canvas,
              Vec2f pos,
              Color color,
              float fontSize,
              const dr4::Font *font,
              std::string initialText = {})
        : canvas_(canvas)
        , pos_(pos)
        , text_(std::move(initialText))
        , color_(color)
        , fontSize_(fontSize)
        , font_(font) {}

    void AppendUtf8(const char *utf8) {
        if (!utf8) return;
        if (hasSelection()) eraseSelection();
        insertAtCaret(utf8);
    }

    void BackspaceUtf8() {
        if (text_.empty()) return;

        if (hasSelection()) {
            eraseSelection();
            return;
        }

        size_t prev = prevCodepoint(caret_);
        if (prev == caret_) return;

        text_.erase(prev, caret_ - prev);
        caret_ = prev;
        selAnchor_ = caret_;
        markDirty();
    }

    const std::string &GetText() const {
        return text_;
    }

    void SetColor(Color c) {
        color_ = c;
        markDirty();
    }

    void SetFontSize(float s) {
        fontSize_ = s;
        markDirty();
    }

    bool HitTest(Vec2f p) const {
        Vec2f size = measureText();
        if (size.x <= 0.f) size.x = fontSize_ * 0.5f;
        if (size.y <= 0.f) size.y = fontSize_;
        size.x += 2 * Padding;
        size.y += 2 * Padding;
        return isInside(p, {pos_.x - Padding, pos_.y - Padding}, size);
    }

    void SetPos(Vec2f pos) override {
        pos_ = pos;
        markDirty();
    }

    Vec2f GetPos() const override {
        return pos_;
    }

    void DrawOn(dr4::Texture &tex) const override {
        if (!canvas_) return;
        dr4::Window *wnd = canvas_->GetWindow();
        if (!wnd) return;

        std::unique_ptr<dr4::Text> t(wnd->CreateText());
        if (!t) return;

        if (font_) t->SetFont(font_);
        t->SetText(text_);
        t->SetColor(color_);
        t->SetFontSize(fontSize_);
        t->SetVAlign(dr4::Text::VAlign::TOP);
        t->SetPos(pos_);

        t->DrawOn(tex);

        cachedBounds_ = t->GetBounds();
        boundsDirty_  = false;

        auto theme = canvas_->GetControlsTheme();

        if (selected_) {
            Vec2f size = cachedBounds_;

            if (size.x <= 0.f) size.x = fontSize_ * 0.5f;
            if (size.y <= 0.f) size.y = fontSize_;

            std::unique_ptr<dr4::Rectangle> r(wnd->CreateRectangle());
            if (r) {
                r->SetPos({pos_.x - Padding, pos_.y - Padding});
                r->SetSize({size.x + 2 * Padding, size.y + 2 * Padding});

                dr4::Color transparent(0, 0, 0, 0);
                r->SetFillColor(transparent);
                r->SetBorderThickness(1.0f);

                dr4::Color borderColor =
                    editing_ ? theme.selectColor
                             : theme.shapeBorderColor;

                r->SetBorderColor(borderColor);

                r->DrawOn(tex);
            }
        }

        if (editing_ && hasSelection()) {
            auto [a, b] = selRange();

            std::unique_ptr<dr4::Text> mt(wnd->CreateText());
            if (mt) {
                if (font_) mt->SetFont(font_);
                mt->SetFontSize(fontSize_);
                mt->SetVAlign(dr4::Text::VAlign::TOP);

                mt->SetText(text_.substr(0, a));
                float x0 = mt->GetBounds().x;

                mt->SetText(text_.substr(0, b));
                float x1 = mt->GetBounds().x;

                float h = cachedBounds_.y > 0.f ? cachedBounds_.y : fontSize_;

                std::unique_ptr<dr4::Rectangle> sel(wnd->CreateRectangle());
                if (sel) {
                    sel->SetPos({pos_.x + x0, pos_.y});
                    sel->SetSize({std::max(0.f, x1 - x0), h});

                    Color c = theme.selectColor;
                    c.a = 90;
                    sel->SetFillColor(c);
                    sel->SetBorderThickness(0.0f);
                    sel->SetBorderColor(c);

                    sel->DrawOn(tex);
                }
            }
        }

        if (editing_ && caretVisible_) {
            Vec2f size = cachedBounds_;
            if (size.y <= 0.f) size.y = fontSize_;

            float caretX = pos_.x + caretOffsetX();
            float caretTop = pos_.y;
            float caretBottom = pos_.y + size.y;

            std::unique_ptr<dr4::Line> caret(wnd->CreateLine());
            if (caret) {
                caret->SetStart({caretX, caretTop});
                caret->SetEnd({caretX, caretBottom});
                caret->SetColor(theme.textColor);
                caret->SetThickness(1.0f);
                caret->DrawOn(tex);
            }
        }
    }

    bool OnMouseDown(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;
        if (evt.button != MBT::LEFT) return false;

        Vec2f size = measureText();
        if (size.x <= 0.f) size.x = fontSize_ * 0.5f;
        if (size.y <= 0.f) size.y = fontSize_;

        size.x += 2 * Padding;
        size.y += 2 * Padding;

        Vec2f boxPos{pos_.x - Padding, pos_.y - Padding};

        if (!isInside(evt.pos, boxPos, size))
            return false;

        if (editing_) {
            caret_ = indexFromX(evt.pos.x);
            selAnchor_ = caret_;
            selectingText_ = true;
            markDirty();
            return true;
        }

        dragging_   = true;
        dragOffset_ = evt.pos - pos_;
        return true;
    }

    bool OnMouseMove(const dr4::Event::MouseMove &evt) override {
        if (selectingText_) {
            caret_ = indexFromX(evt.pos.x);
            markDirty();
            return true;
        }
        if (dragging_) {
            SetPos(evt.pos - dragOffset_);
            return true;
        }
        return false;
    }

    bool OnMouseUp(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;
        if (evt.button != MBT::LEFT) return false;

        if (selectingText_) {
            selectingText_ = false;
            return true;
        }

        if (dragging_) {
            dragging_ = false;
            return true;
        }

        return false;
    }

    bool OnIdle(const pp::IdleEvent &evt) override {
        if (!editing_) return false;

        constexpr double halfPeriod = 0.5;
        double t = evt.absTime;
        bool visible = std::fmod(t, 2.0 * halfPeriod) < halfPeriod;

        if (visible != caretVisible_) {
            caretVisible_ = visible;
            if (canvas_) {
                canvas_->ShapeChanged(this);
            }
        }
        return true;
    }

    void OnSelect() override {
        selected_ = true;
    }

    void OnDeselect() override {
        selected_ = false;
        editing_  = false;
        dragging_ = false;
        caretVisible_ = true;
    }

    void SetEditing(bool e) {
        if (editing_ == e) return;
        editing_ = e;
        if (canvas_) {
            canvas_->ShapeChanged(this);
        }
    }

    void MoveCaretLeft(bool ctrl, bool shift) {
        size_t old = caret_;
        caret_ = ctrl ? prevWord(caret_) : prevCodepoint(caret_);
        caret_ = clampIndex(caret_);
        if (!shift) selAnchor_ = caret_;
        if (old != caret_) markDirty();
    }

    void MoveCaretRight(bool ctrl, bool shift) {
        size_t old = caret_;
        caret_ = ctrl ? nextWord(caret_) : nextCodepoint(caret_);
        caret_ = clampIndex(caret_);
        if (!shift) selAnchor_ = caret_;
        if (old != caret_) markDirty();
    }

    void DeleteSelectionIfAny() {
        if (hasSelection()) eraseSelection();
    }
};

class TextTool final : public pp::Tool {
    pp::Canvas *canvas_{nullptr};
    TextShape  *current_{nullptr};
    TextShape  *dragging_{nullptr};
    bool        editing_{false};

    std::unique_ptr<dr4::Font> font_;

    dr4::Window *window() const {
        return canvas_ ? canvas_->GetWindow() : nullptr;
    }

    const dr4::Font *ensureFont() {
        if (font_) return font_.get();

        auto *w = window();
        if (!w) return nullptr;

        font_.reset(w->CreateFont());
        if (!font_) return nullptr;

        font_->LoadFromBuffer(g_TextToolFontData, g_TextToolFontSize);
        return font_.get();
    }

    static bool hasCtrl(uint16_t mods)  { return (mods & dr4::KEYMOD_CTRL)  != 0; }
    static bool hasShift(uint16_t mods) { return (mods & dr4::KEYMOD_SHIFT) != 0; }

public:
    explicit TextTool(pp::Canvas *cvs) : canvas_(cvs) {}

    std::string_view Icon() const override {
        static constexpr std::string_view icon = u8"󰊄";
        return icon;
    }

    std::string_view Name() const override {
        static constexpr std::string_view name = "Text";
        return name;
    }

    bool IsCurrentlyDrawing() const override {
        return editing_;
    }

    void OnStart() override {}

    void OnEnd() override {
        if (editing_ && current_) {
            current_->SetEditing(false);
        }
        if (editing_) {
            if (current_ && current_->GetText().empty()) {
                canvas_->DelShape(current_);
            }
            editing_ = false;
            current_ = nullptr;
        }

        dragging_ = nullptr;

        if (auto *w = window()) {
            w->StopTextInput();
        }
    }

    void OnBreak() override {
        if (editing_ && current_) {
            current_->SetEditing(false);
        }
        if (editing_) {
            if (current_ && current_->GetText().empty()) {
                canvas_->DelShape(current_);
            }
            editing_ = false;
            current_ = nullptr;
        }

        dragging_ = nullptr;

        if (auto *w = window()) {
            w->StopTextInput();
        }
    }

    bool OnMouseDown(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;

        if (evt.button != MBT::LEFT) return false;

        if (!editing_) {
            if (auto *selected = dynamic_cast<TextShape*>(canvas_->GetSelectedShape())) {
                if (selected->HitTest(evt.pos)) {
                    if (selected->OnMouseDown(evt)) {
                        dragging_ = selected;
                        return true;
                    }
                }
            }
        }

        auto theme = canvas_->GetControlsTheme();

        const dr4::Font *font = ensureFont();

        auto *shape = new TextShape(canvas_,
                                    evt.pos,
                                    theme.textColor,
                                    theme.baseFontSize,
                                    font);

        canvas_->AddShape(shape);
        canvas_->SetSelectedShape(shape);
        canvas_->ShapeChanged(shape);

        current_  = shape;
        editing_  = true;
        shape->SetEditing(true);
        dragging_ = nullptr;

        if (auto *w = window()) {
            w->StartTextInput();
        }

        return true;
    }

    bool OnMouseMove(const dr4::Event::MouseMove &evt) override {
        if (dragging_) {
            dragging_->OnMouseMove(evt);
            return true;
        }
        return false;
    }

    bool OnMouseUp(const dr4::Event::MouseButton &evt) override {
        using MBT = dr4::MouseButtonType;
        if (evt.button != MBT::LEFT) return false;

        if (dragging_) {
            dragging_->OnMouseUp(evt);
            dragging_ = nullptr;
            return true;
        }

        return false;
    }

    bool OnText(const dr4::Event::TextEvent &evt) override {
        if (!editing_ || !current_) return false;
        current_->AppendUtf8(evt.unicode);
        return true;
    }

    bool OnKeyDown(const dr4::Event::KeyEvent &evt) override {
        switch (evt.sym) {
        case dr4::KEYCODE_DELETE: {
            auto *shape = dynamic_cast<TextShape*>(canvas_->GetSelectedShape());
            if (!shape) break;

            if (editing_ && current_ == shape) {
                shape->SetEditing(false);
                editing_ = false;
                current_ = nullptr;
                dragging_ = nullptr;
                if (auto *w = window()) {
                    w->StopTextInput();
                }
            }

            canvas_->DelShape(shape);
            return true;
        }

        case dr4::KEYCODE_ESCAPE: {
            if (editing_) {
                if (current_) {
                    current_->SetEditing(false);
                }
                editing_ = false;
                current_ = nullptr;
                dragging_ = nullptr;
                if (auto *w = window()) {
                    w->StopTextInput();
                }
            }

            if (canvas_->GetSelectedShape()) {
                canvas_->SetSelectedShape(nullptr);
                return true;
            }

            return false;
        }

        default:
            break;
        }

        if (hasCtrl(evt.mods)) {
            if (evt.sym == dr4::KEYCODE_C) {
                auto *shape = dynamic_cast<TextShape*>(canvas_->GetSelectedShape());
                if (!shape) return false;

                if (auto *w = window()) {
                    w->SetClipboard(shape->selectedText());
                }
                return true;
            }

            if (evt.sym == dr4::KEYCODE_X) {
                auto *shape = dynamic_cast<TextShape*>(canvas_->GetSelectedShape());
                if (!shape) return false;

                std::string sel = shape->selectedText();
                if (sel.empty()) return true;

                if (auto *w = window()) {
                    w->SetClipboard(sel);
                }

                if (!editing_) {
                    current_ = shape;
                    editing_ = true;
                    shape->SetEditing(true);
                    if (auto *w = window()) {
                        w->StartTextInput();
                    }
                }

                if (current_ == shape) {
                    shape->DeleteSelectionIfAny();
                }

                return true;
            }

            if (evt.sym == dr4::KEYCODE_V) {
                auto *shape = dynamic_cast<TextShape*>(canvas_->GetSelectedShape());
                if (!shape) return false;

                if (auto *w = window()) {
                    std::string clip = w->GetClipboard();
                    if (!clip.empty()) {
                        if (!editing_) {
                            current_ = shape;
                            editing_ = true;
                            shape->SetEditing(true);
                            w->StartTextInput();
                        }

                        if (current_ == shape) {
                            current_->DeleteSelectionIfAny();
                            current_->AppendUtf8(clip.c_str());
                        }
                    }
                }
                return true;
            }
        }

        if (!editing_) {
            if (evt.sym == dr4::KEYCODE_ENTER || evt.sym == dr4::KEYCODE_F2) {
                if (auto *shape = dynamic_cast<TextShape*>(canvas_->GetSelectedShape())) {
                    current_ = shape;
                    editing_ = true;
                    shape->SetEditing(true);
                    if (auto *w = window()) {
                        w->StartTextInput();
                    }
                    return true;
                }
            }
            return false;
        }

        if (!current_) return false;

        switch (evt.sym) {
            case dr4::KEYCODE_BACKSPACE:
                current_->BackspaceUtf8();
                return true;

            case dr4::KEYCODE_ENTER:
                if (current_) {
                    current_->SetEditing(false);
                }
                editing_ = false;
                current_ = nullptr;
                if (auto *w = window()) {
                    w->StopTextInput();
                }
                return true;

            case dr4::KEYCODE_LEFT: {
                bool ctrl  = hasCtrl(evt.mods);
                bool shift = hasShift(evt.mods);
                current_->MoveCaretLeft(ctrl, shift);
                return true;
            }

            case dr4::KEYCODE_RIGHT: {
                bool ctrl  = hasCtrl(evt.mods);
                bool shift = hasShift(evt.mods);
                current_->MoveCaretRight(ctrl, shift);
                return true;
            }
            default:
                break;
        }

        return false;
    }
};
}

#endif
