#pragma once
#include "graph.hpp"
#include "reactor.hpp"
#include "state.hpp"

class KineticGraph : public LineGraph {
public:
	KineticGraph(SDL_FRect dims, Widget *parent_, const char *label_y_, const char *title_, State *st, double x_axis_shift_ = 0.8, int fps = 60, double time_window_s_ = 5.0)
		: Widget(dims, parent_, st), LineGraph(dims, parent_, label_y_, title_, st, x_axis_shift_, fps, time_window_s_) {}

	DispatchResult on_idle(DispatcherCtx ctx, const IdleEvent *e) {
		(void)ctx;
		(void)e;
		Stat stats = ((ReactorState *)state)->reactor->tally();
		this->append_sample(stats.kinetic);
		this->rescale_y();
		this->snap_y_scale_to_grid();
		return PROPAGATE;
	}
};

class TemperatureGraph : public LineGraph {
public:
	TemperatureGraph(SDL_FRect dims, Widget *parent_, const char *label_y_, const char *title_, State *st, double x_axis_shift_ = 0.8, int fps = 60, double time_window_s_ = 5.0)
		: Widget(dims, parent_, st), LineGraph(dims, parent_, label_y_, title_, st, x_axis_shift_, fps, time_window_s_) {}

	DispatchResult on_idle(DispatcherCtx ctx, const IdleEvent *e) {
		(void)ctx;
		(void)e;
		Stat stats = ((ReactorState *)state)->reactor->tally();
		this->append_sample(stats.right_temperature);
		this->rescale_y();
		this->snap_y_scale_to_grid();
		return PROPAGATE;
	}
};
