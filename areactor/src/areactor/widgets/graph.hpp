#pragma once
#include <SDL3/SDL.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>

#include <swuix/window/window.hpp>
#include <swuix/widgets/handle.hpp>

#include "ring_buffer.hpp"

static double nice125(double x) {
	if (x <= 0) return 0.0;
	const double e = std::floor(std::log10(x));
	const double base = std::pow(10.0, e);
	const double n = x / base;  // ~[1..10)

	double m;
	if      (n < 1.5) m = 1.0;
	else if (n < 3.5) m = 2.0;
	else if (n < 7.5) m = 5.0;
	else {
		m = 1.0;
		return m * std::pow(10.0, e + 1.0);
	}

	return m * base;
}

static void step_125_up(double &m, int &e) {
	if (m < 1.5) m = 2.0;
	else if (m < 3.5) m = 5.0;
	else {
		m = 1.0;
		++e;
	}
}

static void step_125_down(double &m, int &e) {
	if (m < 1.5) {
		m = 5.0;
		--e;
	} else if (m < 3.5) m = 1.0;
	else m = 2.0;
}

static std::string fmt_si(double v, const char *unit = "", int sig = 3) {
	if (v == 0.0) {
		std::ostringstream o; o << "0" << (unit && *unit ? " " : "") << unit; return o.str();
	}
	const double av = std::fabs(v);
	int e3 = (int)std::floor(std::log10(av) / 3.0);
	if (e3 < -8) e3 = -8;
	if (e3 > 8) e3 = 8;

	static const char *prefix[] = {
		//                               micro
		"y", "z", "a", "f", "p", "n", "\xC2\xB5", "m", "", "k", "M", "G", "T", "P", "E", "Z", "Y"
	};
	double scaled = v / std::pow(10.0, 3.0 * e3);

	int digits = sig;
	int intd = (int)std::floor(std::log10(std::fabs(scaled) + 1e-12)) + 1;
	if (intd < 1) intd = 1;
	int dec = digits - intd; if (dec < 0) dec = 0;

	std::ostringstream o;
	o.setf(std::ios::fixed); o << std::setprecision(dec) << scaled;
	o << " " << prefix[e3 + 8] << unit;
	return o.str();
}

static std::string fmt_seconds(double t) {
	std::ostringstream o;
	// -0.0 -> 0.0:
	if (std::fabs(t) < 0.1) t = 0.0;
	o.setf(std::ios::fixed);
	o << std::setprecision(0) << t << "s";
	return o.str();
}

struct AutoClip {
	SDL_Renderer *r;
	SDL_Rect prev;

	AutoClip(SDL_Renderer *rr, const SDL_FRect &f) : r(rr) {
		SDL_GetRenderClipRect(r, &prev);
		SDL_Rect clip = {
			(int)SDL_floorf(f.x),
			(int)SDL_floorf(f.y),
			(int)SDL_ceilf (f.w),
			(int)SDL_ceilf (f.h + 20)
		};
		SDL_SetRenderClipRect(r, &clip);
	}

	~AutoClip() {
		SDL_SetRenderClipRect(r, NULL);
	}
};

// Cohen–Sutherland algorithm (clipping lines)

enum {
	CS_INSIDE = 0,
	CS_LEFT   = 1 << 0,
	CS_RIGHT  = 1 << 1,
	CS_BOTTOM = 1 << 2,
	CS_TOP    = 1 << 3
};

static inline int cs_outcode(double x, double y, const SDL_FRect *r) {
	int code = CS_INSIDE;

	if (x < r->x) code |= CS_LEFT;
	else if (x > r->x + r->w) code |= CS_RIGHT;

	if (y < r->y) code |= CS_TOP;
	else if (y > r->y + r->h) code |= CS_BOTTOM;

	return code;
}

// return true if something remains
static bool clip_to_rect(double *x0, double *y0, double *x1, double *y1, const SDL_FRect *r) {
	int out0 = cs_outcode(*x0, *y0, r);
	int out1 = cs_outcode(*x1, *y1, r);

	while (1) {
		if (!(out0 | out1)) return true;  // trivially accept
		if (out0 & out1) return false;    // trivially reject

		// pick an endpoint outside the rect
		int out = out0 ? out0 : out1;
		double x, y;

		if (out & CS_TOP) { // y < top
			x = *x0 + (*x1 - *x0) * (r->y - *y0) / (*y1 - *y0);
			y = r->y;
		} else if (out & CS_BOTTOM) { // y > bottom
			x = *x0 + (*x1 - *x0) * ((r->y + r->h) - *y0) / (*y1 - *y0);
			y = r->y + r->h;
		} else if (out & CS_RIGHT) { // x > right
			y = *y0 + (*y1 - *y0) * ((r->x + r->w) - *x0) / (*x1 - *x0);
			x = r->x + r->w;
		} else { // CS_LEFT: x < left
			y = *y0 + (*y1 - *y0) * (r->x - *x0) / (*x1 - *x0);
			x = r->x;
		}

		if (out == out0) {
			*x0 = x;
			*y0 = y;
			out0 = cs_outcode(*x0, *y0, r);
		} else {
			*x1 = x;
			*y1 = y;
			out1 = cs_outcode(*x1, *y1, r);
		}
	}
}

struct Axis {
	double center;  // absolute
	double scale;   // pixels per unit
};

static inline int pos_mod(int a, int b) {
	return (a % b + b) % b;
}

class LineGraph : public HandledWidget {
	double UNITS_PER_SECONDS;
	int FPS;
	double time_window_s;
	size_t sample_count;
	RingBuffer<double> buf;
	const char *label_y, *grap_title;

	const char *title() const {
		return "LineGraph";
	}

	void draw_grid(Window *window, double x_units, const char *y_unit) {
		int x_step_px, y_step_px;
		double y_units;
		calc_125_scale(x_step_px, x_units, y_step_px, y_units);

		int k0 = (int)SDL_ceil((frame.y - x_axis.center) / (double)y_step_px);
		int k1 = (int)SDL_floor((frame.y + frame.h - x_axis.center) / (double)y_step_px);

		for (int k = k0; k <= k1; ++k) {
			float ys = (float)(x_axis.center + k * y_step_px);
			window->draw_line_rgb(frame.x, ys, frame.x + frame.w, ys, 1, CLR_GRID);

			const double v = (double)k * y_units;
			window->text_aligned(fmt_si(-v, y_unit).c_str(), frame.x - 6.0f, ys, TA_RIGHT);
		}

		AutoClip clipper(window->renderer, frame);

		k0 = (int)SDL_ceil((frame.x - y_axis.center) / (double)x_step_px);
		k1 = (int)SDL_floor((frame.x + frame.w - y_axis.center) / (double)x_step_px);

		const double t_at_yaxis = x_screen_to_space(y_axis.center);

		for (int k = k0; k <= k1; ++k) {
			float xs = (float)(y_axis.center + k * x_step_px);
			window->draw_line_rgb(xs, frame.y, xs, frame.y + frame.h, 1, CLR_GRID);

			const double t_abs = t_at_yaxis + k * x_units;
			window->text_aligned(fmt_seconds(t_abs).c_str(), xs, frame.y + frame.h + 10.0f, TA_CENTER);
		}
	}

	void draw_axis_titles(Window *win, const char *x_title, const char *y_title) {
		win->text_aligned(x_title, frame.x + frame.w * 0.5f, frame.y + frame.h + 18.0f, TA_CENTER);
		win->text_aligned(y_title, frame.x + frame.w + 8.0f, frame.y + frame.h * 0.5f, TA_LEFT);
	}

	void draw_axes(Window *window) {
		float y = (float)std::min(std::max((float)x_axis.center, frame.y), frame.y + frame.h);
		window->draw_line(frame.x, y, frame.x + frame.w, y, 2);

		float x = (float)std::min(std::max((float)y_axis.center, frame.x), frame.x + frame.w);
		window->draw_line(x, frame.y, x, frame.y + frame.h, 2);
	}

	void calc_125_scale(int &x_step_px, const double &x_units, int &y_step_px, double &y_units) {
		x_step_px = (int)SDL_round(x_axis.scale * x_units);
		if (x_step_px <= 0) return;

		// want Y pixel step ~ X pixel step
		double raw_y_units = (double)x_step_px / (double)y_axis.scale;
		y_units = nice125(raw_y_units);

		// Clamp Y px step to [0.5, 1.5] * x_step_px with 1-2-5 stepping (hysteresis)
		y_step_px = (int)SDL_round(y_units * y_axis.scale);
		const int lo  = (int)SDL_floor(0.5 * x_step_px);
		const int hi  = (int)SDL_ceil (1.5 * x_step_px);

		int ye = (int)std::floor(std::log10(y_units));     // exponent
		double ym = y_units / std::pow(10.0, (double)ye);  // mantissa

		int limit = 200;
		while (y_step_px < lo && limit > 0) {
			step_125_up(ym, ye);
			y_units = ym * std::pow(10.0, (double)ye);
			y_step_px = (int)SDL_round(y_units * y_axis.scale);
			limit--;
		}
		limit = 200;
		while (y_step_px > hi && limit > 0) {
			step_125_down(ym, ye);
			y_units = ym * std::pow(10.0, (double)ye);
			y_step_px = (int)SDL_round(y_units * y_axis.scale);
			limit--;
		}
	}
public:
	Axis x_axis;
	Axis y_axis;
	double x_axis_shift;

	LineGraph(SDL_FRect dims, Widget *parent_, const char *label_y_, const char *title_, State *st, double x_axis_shift_ = 0.8, int fps = 60, double time_window_s_ = 5.0)
			: Widget(dims, parent_, st), HandledWidget(dims, parent_, st), UNITS_PER_SECONDS(1.0), FPS(fps), time_window_s(time_window_s_),
			sample_count(0), buf(4096), label_y(label_y_), grap_title(title_), x_axis_shift(x_axis_shift_) {
		x_axis.center = frame.y + frame.h * x_axis_shift;
		x_axis.scale  = frame.w / time_window_s * UNITS_PER_SECONDS;
		y_axis.center = frame.x;
		y_axis.scale  = 1;
	}

	void pin_to_right(double x_latest) {
		y_axis.center = frame.x + frame.w - x_latest * (double)x_axis.scale;
	}

	double x_screen_to_space(double x_screen) const {
		return (x_screen - y_axis.center) / (double)x_axis.scale;
	}

	double y_screen_to_space(double y_screen) const {
		return (x_axis.center - y_screen) / (double)y_axis.scale;
	}

	double x_space_to_screen(double x_space) const {
		return y_axis.center + x_space * (double)x_axis.scale;
	}

	double y_space_to_screen(double y_space) const {
		return x_axis.center - y_space * (double)y_axis.scale;
	}

	void space_to_screen(Point2f *vec) const {
		vec->x = x_space_to_screen(vec->x);
		vec->y = y_space_to_screen(vec->y);
	}

	void screen_to_space(Point2f *vec) const {
		vec->x = x_screen_to_space(vec->x);
		vec->y = y_screen_to_space(vec->y);
	}

	void append_sample(double sample) {
		buf.push(sample);
		sample_count++;

		const double dx = (frame.w / x_axis.scale) / (FPS * time_window_s);

		double x_latest = (double)sample_count * dx;

		pin_to_right(x_latest);
	}

	void rescale_y(double factor = 2.0) {
		y_axis.scale = frame.h / buf.mean((int)((double)FPS * time_window_s)) / factor;
	}

	void snap_y_scale_to_grid(double x_units = 1.0) {
		int x_step_px, y_step_px;
		double y_units;
		calc_125_scale(x_step_px, x_units, y_step_px, y_units);

		y_axis.scale = (double)y_step_px / y_units;
	}

	void plot_stream(SDL_Renderer *renderer) {
		const double dx = (frame.w / x_axis.scale) / (FPS * time_window_s);
		double x_latest = (double)sample_count * dx;

		const size_t n = buf.size;
		const double x0_base = x_latest - (n - 1) * dx; // space x of oldest sample stored

		if (n < 2) return;

		const double w_space = frame.w / x_axis.scale;
		size_t max_need = (size_t)w_space / dx + 3;  // just to be safe
		if (max_need > n) max_need = n;

		size_t start = n - max_need;
		double x_prev_space = x0_base + (double)start * dx;

		double x0 = x_space_to_screen(x_prev_space);
		double y0 = y_space_to_screen(buf.at(start));

		SDL_SetRenderDrawColor(renderer, CLR_PUCE, SDL_ALPHA_OPAQUE);
		for (size_t i = start + 1; i < n; ++i) {
			double x1_space = x0_base + (double)i * dx;
			double x1 = x_space_to_screen(x1_space);
			double y1 = y_space_to_screen(buf.at(i));

			double cx0 = x0, cy0 = y0, cx1 = x1, cy1 = y1;
			if (clip_to_rect(&cx0, &cy0, &cx1, &cy1, &frame)) {
				SDL_RenderLine(renderer, (float)cx0, (float)cy0, (float)cx1, (float)cy1);
			}

			x0 = x1; y0 = y1;
		}
	}

	void render_body(Window *window, int off_x, int off_y) {
		window->clear_rect(frame, off_x, off_y, CLR_TIMBERWOLF);
		draw_grid(window, 1.0, label_y);
		draw_axes(window);
		plot_stream(window->renderer);
		draw_axis_titles(window, "", grap_title);
		window->outline(frame, off_x, off_y, 2);
	}

	DispatchResult on_mouse_move(DispatcherCtx ctx, const MouseMoveEvent *e) {
		x_axis.center = frame.y + frame.h * x_axis_shift;
		return HandledWidget::on_mouse_move(ctx, e);
	}
};
