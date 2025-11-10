#pragma once
#include <swuix/traits/minimizable.hpp>
#include <swuix/traits/draggable.hpp>
#include <swuix/widgets/button.hpp>

const float HANDLE_H = 20.0f;

class TitleBar final : public DraggableWidget {
    Button *btn_minimize;
    MinimizableWidget *host;

public:
    TitleBar(State*);

    const char *title() const override {
        return "Titlebar";
    }

    bool isClipped() const override { return false; }

    void attachTo(MinimizableWidget*);

    DispatchResult onMouseMove(DispatcherCtx, const MouseMoveEvent *) override;

    void layout() override;
    void draw() override;
};
