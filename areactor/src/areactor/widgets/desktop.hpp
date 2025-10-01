#pragma once
#include <swuix/widgets/container.hpp>

#include "reactor.hpp"
#include "widgets/clock.hpp"
#include "widgets/graphs-misc.hpp"
#include "widgets/toolbox.hpp"

class Desktop : public WidgetContainer {
public:
	Reactor *reactor;

	Desktop(SDL_FRect frame_, Widget *parent_, int fps, State *state_)
			: Widget(frame_, parent_, state_), WidgetContainer(frame_, parent_, state_) {

		LineGraph     *kinetic     = new KineticGraph(frect(1000, 140, 180, 180), NULL, "J", "E", state, 0.8, fps, 5.0);
		LineGraph     *temperature = new TemperatureGraph(frect(1000, 380, 180, 180), NULL, "K", "T", state, 0.8, fps, 5.0);
		ToolboxWidget *toolbox     = new ToolboxWidget(frect(10, 30, 150, 110), NULL, state);
		Clock         *clock       = new Clock(frect(1180, 620, 100, 100), NULL, state);
		               reactor     = new Reactor(frect(180, 180, 640, 380), NULL, 1000, state);

		Widget *arr[] = { clock, toolbox, reactor, kinetic, temperature };
		this->append_children(Widget::make_children(arr));

		((ReactorState*)state)->reactor = reactor;
		this->parent = this;
	}

	const char *title() const {
		return "Desktop";
	}

	void render(Window *window, int off_x, int off_y) {
		window->clear();
		WidgetContainer::render(window, off_x, off_y);
		window->present();
	}
};
