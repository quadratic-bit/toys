#pragma once
#include <cassert>

#include <swuix/traits/scrollable.hpp>
#include <swuix/widgets/scrollbar.hpp>
#include <swuix/widgets/button.hpp>

#include "../trace/objects.hpp"
#include "./objview.hpp"

class ObjectPreview;
class ObjectsList;

class Select : public Action {
    ObjectPreview *preview;
    ObjectsList   *list;

public:
    Select(ObjectPreview *p, ObjectsList *l) : preview(p), list(l) {}
    void apply(void *, Widget *);
};

class ObjectPreview final : public Widget {
    friend class ObjectsList;
    friend class ObjectView;
    friend class ObjectViewName;

    Object *obj;

    Button *selectBtn_ = nullptr;

public:
    ObjectPreview(Object *o, ObjectsList *l, Rect2f f, Widget *p, State *s)
        : Widget(f, p, s), obj(o)
    {
        selectBtn_ = new Button(
            {f.size.x - 75, 5, 70, f.size.y - 10},
            nullptr,
            "Select",
            state,
            new Select(this, l)
        );
        appendChild(selectBtn_);
    }

	const char *title() const override {
		return obj->name.c_str();
	}

    void layout() override {
        if (!selectBtn_) return;
        Rect2f f = frame();

        selectBtn_->position = {f.size.x - 75.0f, 5.0f};
        selectBtn_->texture->SetSize({70.0f, f.size.y - 10.0f});
        selectBtn_->requestLayout();
    }

    void draw() override {
        Rect2f f = frame();
        Rectangle *r;
        if (obj->selected()) {
            r = rectBorder(state->window, f, {CLR_PRIMARY_SURF}, 2, {CLR_PRIMARY});
        } else {
            r = rectBorder(state->window, f, {CLR_SURFACE_1}, 2, {CLR_BORDER});
        }
        texture->Draw(*r);

        Text *t = textAligned(state->window, title(), {5, f.size.y / 2}, Color(CLR_TEXT_STRONG), state->appfont);
        texture->Draw(*t);
    }
};

class Desktop;

class ObjectsList final : public ScrollableWidget {
    const std::vector<Object*> &objects;
    Desktop *root;
    ObjectPreview *selected = nullptr;

    VScrollbar *scrollbar_ = nullptr;

public:
    ObjectsList(const std::vector<Object*> &objects_, Desktop *d, Rect2f f, Vec2f clip, Widget *p, State *s)
        : Widget(Rect2f(f.pos, clip), p, s)
        , ScrollableWidget(f, clip, p, s)
        , objects(objects_)
        , root(d)
    {
        for (size_t i = 0; i < objects.size(); ++i) {
            auto *objPrev = new ObjectPreview(objects[i], this, {5, 5.0f + 35.0f * (float)i, clip.x - 20.0f, 30}, nullptr, state);
            appendChild(objPrev);
        }

        // width should match viewport width
        texture->SetSize({clip.x, std::max(clip.y, 10.0f)});

        scrollbar_ = new VScrollbar(state);
        scrollbar_->attachTo(this);

        requestLayout();
        requestRedraw();
    }

    ObjectPreview *selectedPreview() const { return selected; }

    void toggleSelect(ObjectPreview *preview);

    const char *title() const override { return "Objects"; }

    void layout() override {
        const float pad = 5.0f;
        const float rowH = 30.0f;
        const float rowStep = 35.0f;

        const float viewportW = viewport->GetWidth();
        const float viewportH = viewport->GetHeight();

        // leave space for the scrollbar on the right
        const float itemW = std::max(40.0f, viewportW - 2.0f * pad - SCROLLBAR_W);

        float y = pad;
        for (Widget *c : children) {
            auto *op = dynamic_cast<ObjectPreview*>(c);
            if (!op) continue;

            op->position = {pad, y};
            op->texture->SetSize({itemW, rowH});
            op->layout();

            y += rowStep;
        }

        float contentH = y + pad;
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
        texture->Clear({CLR_SURFACE_1});
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_1}, 2, {CLR_BORDER});
        texture->Draw(*r);
    }
};

class ObjectViewName final : public TextInput {
    ObjectPreview *prev;

public:
    ObjectViewName(ObjectPreview *preview, Rect2f f, Widget *p, State *s)
        : Widget(f, p, s)
        , FocusableWidget(f, p, s)
        , TextInput(f, p, s)
        , prev(preview)
    {
        setText(preview->obj->name);
    }

    const char *title() const override {
        return "ObjectView name edit";
    }

    void onValueChange() override {
        prev->obj->name = value;
        prev->requestRedraw();
    }

    void draw() override {
        Rect2f f = frame();
        Rectangle *r = rectBorder(state->window, f, {CLR_SURFACE_2}, 2, {CLR_BORDER});
        texture->Draw(*r);

        Text *t = textAligned(state->window, getText().c_str(), {5, f.size.y / 2}, Color(CLR_TEXT_STRONG), state->appfont);
        texture->Draw(*t);
    }
};

inline void ObjectView::populate(ObjectPreview *prev) {
    unpopulate();

    Object *obj = prev->obj;

    ObjectViewName *objname = new ObjectViewName(prev, {5, 5, 125, 24}, NULL, state);
    this->appendChild(objname);

    Rect2f f = frame();
    Rect2f list_frame {
        8,
        60,
        f.size.x - 16,
        f.size.y - 68
    };

    ObjectViewPropertyList *objprops = new ObjectViewPropertyList(obj, list_frame, list_frame.size, nullptr, state);
    this->appendChild(objprops);

    requestRedraw();
}

inline void Select::apply(void *, Widget *) {
    list->toggleSelect(preview);
}
