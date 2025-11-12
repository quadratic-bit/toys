#include <swuix/widgets/titlebar.hpp>
#include <swuix/state.hpp>

static const int TITLEBAR_THICK = 2;

class ToggleMinimize : public Action {
    MinimizableWidget *target;
public:
    ToggleMinimize(MinimizableWidget *w) : target(w) {}

    void apply(void*, Widget*) {
        target->minimized ^= true;
    }
};

class Close : public Action {
    MinimizableWidget *target;
public:
    Close(MinimizableWidget *w) : target(w) {}

    void apply(void*, Widget*) {
        target->destroy();
    }
};

static float BTN_W = 15, BTN_H = 10;

static inline const Rect2f handleBoxZero() {
    return {0, -HANDLE_H, 0, HANDLE_H};
}

static inline const Rect2f handleBox(Vec2f parent_box) {
    return {0, -HANDLE_H, parent_box.x, HANDLE_H};
}

TitleBar::TitleBar(State *s) :
        Widget(handleBoxZero(), nullptr, s),
        DraggableWidget(handleBoxZero(), nullptr, s),
        host(nullptr) {
    // in the ctor, width of titlebar is 0, so no point in computing x
    btn_minimize = new Button({0, 5, BTN_W, BTN_H}, this, "-", state);
    btn_close = new Button({0, 5, BTN_W, BTN_H}, this, "x", state);
    this->appendChild(btn_minimize);
    this->appendChild(btn_close);
}

DispatchResult TitleBar::onMouseMove(DispatcherCtx ctx, const MouseMoveEvent *e) {
    if (state->mouse.state == Mouse::State::Dragging && is_dragging) {
        host->translate({
            host->position.x + ctx.mouse_rel.x - start_drag_x,
            host->position.y + ctx.mouse_rel.y - start_drag_y
        });
        host->parent->requestRedraw();
    }
    return Widget::onMouseMove(ctx, e);
}

void TitleBar::layout() {
    btn_minimize->position.x = texture->GetWidth() - BTN_W * 2 - 10;
    btn_close->position.x = texture->GetWidth() - BTN_W - 5;
}

void TitleBar::attachTo(MinimizableWidget *minimizable) {
    host = minimizable;
    minimizable->appendChild(this);
    btn_minimize->setAction(new ToggleMinimize(host));
    btn_close->setAction(new Close(host));

    Rect2f new_box = handleBox(minimizable->texture->GetSize());
    position = new_box.pos;
    texture->SetSize(new_box.size);

    requestLayout();
    requestRedraw();
}

void TitleBar::draw() {
    Rect2f f = frame();
    Rectangle *r = rectBorder(state->window, f, {CLR_PRIMARY_ACT}, TITLEBAR_THICK, {CLR_BORDER});
    texture->Draw(*r);

    Text *t = textAligned(state->window, host->title(), {3, f.size.y / 2}, {CLR_ON_PRIMARY}, state->appfont);
    texture->Draw(*t);
}
