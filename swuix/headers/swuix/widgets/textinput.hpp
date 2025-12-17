#pragma once
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include <swuix/traits/focusable.hpp>
#include <swuix/state.hpp>
#include <swuix/window/common.hpp> // for dr4::Text / dr4::Line helpers

class TextInput : public virtual FocusableWidget {
protected:
    std::string value;

    // caret + selection (UTF-8 byte indices)
    size_t caret_     = 0;
    size_t selAnchor_ = 0;
    bool   selecting_ = false;

    // caret blink
    bool   caretVisible_ = true;

    // horizontal scroll so caret stays visible
    float  scrollX_ = 0.0f;

    // ----- styling hooks (override if needed) -----
    virtual const dr4::Font *textFont() const { return state ? state->appfont : nullptr; }
    virtual float textFontSize() const { return 16.0f; }
    virtual float textInsetX() const { return 5.0f; }   // left padding inside the box
    virtual float caretPadY() const { return 4.0f; }    // top/bottom padding for caret line

    // ----- utf8 helpers -----
    static bool isCont(uint8_t c) { return (c & 0xC0u) == 0x80u; }

    size_t clampIndex(size_t i) const { return std::min(i, value.size()); }

    size_t prevCodepoint(size_t i) const {
        if (i == 0) return 0;
        i = std::min(i, value.size());
        do { --i; } while (i > 0 && isCont((uint8_t)value[i]));
        return i;
    }

    size_t nextCodepoint(size_t i) const {
        i = std::min(i, value.size());
        if (i >= value.size()) return value.size();
        ++i;
        while (i < value.size() && isCont((uint8_t)value[i])) ++i;
        return i;
    }

    static bool isAsciiWordChar(unsigned char c) {
        return std::isalnum(c) || c == '_';
    }

    size_t prevWord(size_t i) const {
        i = std::min(i, value.size());
        if (i == 0) return 0;

        i = prevCodepoint(i);

        while (i > 0 && std::isspace((unsigned char)value[i]))
            i = prevCodepoint(i);

        while (i > 0) {
            size_t pi = prevCodepoint(i);
            if (std::isspace((unsigned char)value[pi])) break;
            i = pi;
        }
        return i;
    }

    size_t nextWord(size_t i) const {
        i = std::min(i, value.size());

        while (i < value.size() && std::isspace((unsigned char)value[i]))
            i = nextCodepoint(i);

        while (i < value.size() && !std::isspace((unsigned char)value[i]))
            i = nextCodepoint(i);

        while (i < value.size() && std::isspace((unsigned char)value[i]))
            i = nextCodepoint(i);

        return i;
    }

    bool hasSelection() const { return selAnchor_ != caret_; }

    std::pair<size_t,size_t> selRange() const {
        size_t a = selAnchor_, b = caret_;
        if (a > b) std::swap(a, b);
        return {a, b};
    }

    void clearSelection() { selAnchor_ = caret_; }

    void eraseSelection() {
        if (!hasSelection()) return;
        auto [a, b] = selRange();
        value.erase(a, b - a);
        caret_ = a;
        selAnchor_ = a;
    }

    // ----- text measurement -----
    float measureWidth(std::string_view s) const {
        if (!state || !state->window) return 0.0f;

        std::unique_ptr<dr4::Text> t(state->window->CreateText());
        if (!t) return 0.0f;

        if (auto *f = textFont()) t->SetFont(f);
        t->SetFontSize(textFontSize());
        t->SetVAlign(dr4::Text::VAlign::TOP);

        t->SetText(std::string(s));
        return t->GetBounds().x;
    }

    size_t indexFromX(float localX) const {
        float x = localX - textInsetX() + scrollX_;
        if (x <= 0.0f) return 0;

        size_t i = 0;
        size_t last = 0;
        while (i < value.size()) {
            size_t ni = nextCodepoint(i);
            float w = measureWidth(std::string_view(value.data(), ni));
            if (w >= x) return ni;
            last = ni;
            i = ni;
        }
        return last;
    }

    void insertAtCaret(std::string_view utf8) {
        if (hasSelection()) eraseSelection();
        value.insert(caret_, utf8);
        caret_ += utf8.size();
        selAnchor_ = caret_;
    }

    void backspaceAtCaret() {
        if (hasSelection()) { eraseSelection(); return; }
        if (caret_ == 0) return;

        size_t prev = prevCodepoint(caret_);
        value.erase(prev, caret_ - prev);
        caret_ = prev;
        selAnchor_ = caret_;
    }

    void deleteAtCaret() {
        if (hasSelection()) { eraseSelection(); return; }
        if (caret_ >= value.size()) return;

        size_t ni = nextCodepoint(caret_);
        value.erase(caret_, ni - caret_);
        selAnchor_ = caret_;
    }

    void ensureCaretVisible() {
        float innerW = (float)texture->GetWidth() - textInsetX() - 4.0f;
        innerW = std::max(10.0f, innerW);

        float cx = caretXpx();
        float leftVis  = scrollX_;
        float rightVis = scrollX_ + innerW;

        if (cx < leftVis) scrollX_ = cx;
        else if (cx > rightVis) scrollX_ = cx - innerW;

        scrollX_ = std::max(0.0f, scrollX_);
    }

    bool wasFocused_ = false;
    void onFocusLost_() {
        selecting_ = false;
        caretVisible_ = true;
        clearSelection();
        requestRedraw();
    }

    static bool hasCtrl(uint16_t mods)  { return (mods & dr4::KEYMOD_CTRL)  != 0; }
    static bool hasShift(uint16_t mods) { return (mods & dr4::KEYMOD_SHIFT) != 0; }

public:
    TextInput(Rect2f f, Widget *p, State *s) : Widget(f, p, s) {}

    const std::string &getText() const { return value; }

    void setText(const std::string &new_value) {
        value = new_value;
        caret_ = std::min(caret_, value.size());
        selAnchor_ = std::min(selAnchor_, value.size());
        ensureCaretVisible();
        requestRedraw();
    }

    // for relayout logic
    float textWidthPx() const { return measureWidth(value); }
    float prefixWidthPx(size_t i) const {
        i = std::min(i, value.size());
        return measureWidth(std::string_view(value.data(), i));
    }
    float caretXpx() const { return prefixWidthPx(caret_); }
    float scrollX() const { return scrollX_; }
    bool  caretVisible() const { return caretVisible_; }
    bool  selectionActive() const { return hasSelection(); }
    std::pair<size_t,size_t> selectionRange() const { return selRange(); }
    size_t caretIndex() const { return caret_; }

    std::string selectedText() const {
        if (!hasSelection()) return {};
        auto [a, b] = selRange();
        return value.substr(a, b - a);
    }

    virtual void onValueChange() = 0;

    DispatchResult onMouseDown(DispatcherCtx ctx, const MouseDownEvent *e) override {
        FocusableWidget::onMouseDown(ctx, e);
        if (!containsMouse(ctx)) return PROPAGATE;

        // place caret
        float localX = ctx.mouse_rel.x - position.x;
        size_t idx = indexFromX(localX);

        caret_ = idx;
        selAnchor_ = caret_;

        selecting_ = true;
        ensureCaretVisible();
        requestRedraw();
        return CONSUME;
    }

    DispatchResult onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) override {
        if (selecting_) {
            float localX = ctx.mouse_rel.x - position.x;
            caret_ = indexFromX(localX);
            caret_ = clampIndex(caret_);
            ensureCaretVisible();
            requestRedraw();
            return CONSUME;
        }
        return Widget::onMouseMove(ctx, e);
    }

    DispatchResult onMouseUp(DispatcherCtx ctx, const MouseUpEvent *e) override {
        (void)ctx; (void)e;
        if (selecting_) {
            selecting_ = false;
            return CONSUME;
        }
        return PROPAGATE;
    }

    DispatchResult onInput(DispatcherCtx, const InputEvent *e) override {
        if (state->getFocus() != this) return PROPAGATE;
        if (!e || !e->text) return PROPAGATE;

        insertAtCaret(e->text);
        ensureCaretVisible();
        onValueChange();
        requestRedraw();
        return CONSUME;
    }

    DispatchResult onKeyDown(DispatcherCtx, const KeyDownEvent *e) override {
        if (state->getFocus() != this) return PROPAGATE;
        if (!e) return PROPAGATE;

        const bool ctrl  = hasCtrl(e->mods);
        const bool shift = hasShift(e->mods);

        // clipboard / selection shortcuts
        if (ctrl) {
            switch (e->keycode) {
                case dr4::KEYCODE_A:
                    selAnchor_ = 0; caret_ = value.size();
                    ensureCaretVisible(); requestRedraw();
                    return CONSUME;

                case dr4::KEYCODE_C: {
                    if (auto *w = state->window) w->SetClipboard(selectedText());
                    return CONSUME;
                }

                case dr4::KEYCODE_X: {
                    if (auto *w = state->window) w->SetClipboard(selectedText());
                    if (hasSelection()) {
                        eraseSelection();
                        ensureCaretVisible();
                        onValueChange();
                        requestRedraw();
                    }
                    return CONSUME;
                }

                case dr4::KEYCODE_V: {
                    if (auto *w = state->window) {
                        std::string clip = w->GetClipboard();
                        if (!clip.empty()) {
                            insertAtCaret(clip);
                            ensureCaretVisible();
                            onValueChange();
                            requestRedraw();
                        }
                    }
                    return CONSUME;
                }

                default: break;
            }
        }

        switch (e->keycode) {
            case dr4::KEYCODE_BACKSPACE:
                backspaceAtCaret();
                ensureCaretVisible();
                onValueChange();
                requestRedraw();
                return CONSUME;

            case dr4::KEYCODE_DELETE:
                deleteAtCaret();
                ensureCaretVisible();
                onValueChange();
                requestRedraw();
                return CONSUME;

            case dr4::KEYCODE_LEFT: {
                size_t old = caret_;
                caret_ = ctrl ? prevWord(caret_) : prevCodepoint(caret_);
                caret_ = clampIndex(caret_);
                if (!shift) selAnchor_ = caret_;
                if (caret_ != old) {
                    ensureCaretVisible();
                    requestRedraw();
                }
                return CONSUME;
            }

            case dr4::KEYCODE_RIGHT: {
                size_t old = caret_;
                caret_ = ctrl ? nextWord(caret_) : nextCodepoint(caret_);
                caret_ = clampIndex(caret_);
                if (!shift) selAnchor_ = caret_;
                if (caret_ != old) {
                    ensureCaretVisible();
                    requestRedraw();
                }
                return CONSUME;
            }

            case dr4::KEYCODE_HOME:
                caret_ = 0;
                if (!shift) selAnchor_ = caret_;
                ensureCaretVisible();
                requestRedraw();
                return CONSUME;

            case dr4::KEYCODE_END:
                caret_ = value.size();
                if (!shift) selAnchor_ = caret_;
                ensureCaretVisible();
                requestRedraw();
                return CONSUME;

            default:
                break;
        }

        return PROPAGATE;
    }

    DispatchResult onIdle(DispatcherCtx, const IdleEvent *) override {
        bool focused = (state->getFocus() == this);

        // focus transition: focused -> not focused
        if (wasFocused_ && !focused) {
            onFocusLost_();
        }

        // caret blink only while focused
        if (focused) {
            double t = state->window ? state->window->GetTime() : 0.0;
            bool v = std::fmod(t, 1.0) < 0.5;
            if (v != caretVisible_) {
                caretVisible_ = v;
                requestRedraw();
            }
        } else {
            if (!caretVisible_) {
                caretVisible_ = true;
                requestRedraw();
            }
        }

        wasFocused_ = focused;
        return PROPAGATE;
    }
};
