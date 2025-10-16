#pragma once
#include <ctime>
#include <iomanip>
#include <sstream>

#include <swuix/window/window.hpp>
#include <swuix/widgets/handle.hpp>

#include "linalg/vectors.hpp"

class Clock : public TitledWidget {
public:
    time_t time_now;

    Clock(Rect2F rect, Widget *parent_, State *s)
        : Widget(rect, parent_, s), TitledWidget(rect, parent_, s) {
            time(&time_now);
        }

    const char *title() const {
        return "Clock";
    }

    DispatchResult on_idle(DispatcherCtx ctx, const IdleEvent *e) {
        (void)e;
        (void)ctx;
        time(&time_now);
        return PROPAGATE;
    }

    void render(Window *window, float off_x, float off_y) {
        window->clear_rect(frame, off_x, off_y, CLR_PLATINUM);
        window->outline(frame, off_x, off_y, 2);
        window->draw_circle_rgb(frame.x + off_x + frame.w / 2, frame.y + off_y + frame.h / 2, 45, CLR_NIGHT);
        struct tm* local_time = localtime(&time_now);

        std::ostringstream oss;
        oss     << std::setfill('0') << std::setw(2) << local_time->tm_hour << ':'
            << std::setfill('0') << std::setw(2) << local_time->tm_min << ':'
            << std::setfill('0') << std::setw(2) << local_time->tm_sec;

        std::string str_time = oss.str();
        window->text_aligned(str_time.c_str(), frame.x + off_x + frame.w / 2, frame.y + frame.h - 10, TA_CENTER);

        float hourf = (float)(local_time->tm_hour % 12) + (float)local_time->tm_min / 60.0f + (float)local_time->tm_sec / 3600.0f;
        float minf  = (float)local_time->tm_min + (float)local_time->tm_sec / 60.0f;
        float secf  = (float)local_time->tm_sec;

        float hour_angle = (float)M_PI * 2 * hourf / 12.0f;
        float min_angle  = (float)M_PI * 2 * minf  / 60.0f;
        float sec_angle  = (float)M_PI * 2 * secf  / 60.0f;

        Vec2f arrow_hour(0, -20);
        Vec2f arrow_min (0, -30);
        Vec2f arrow_sec (0, -40);

        arrow_hour = arrow_hour.rotate(hour_angle);
        arrow_min  = arrow_min.rotate(min_angle);
        arrow_sec  = arrow_sec.rotate(sec_angle);

        Vec2f offset = Vec2f(frame.x + off_x + frame.w / 2, frame.y + off_y + frame.h / 2);
        arrow_hour += offset;
        arrow_min  += offset;
        arrow_sec  += offset;

        window->draw_line(offset.x, offset.y, arrow_hour.x, arrow_hour.y, 3);
        window->draw_line(offset.x, offset.y,  arrow_min.x,  arrow_min.y, 3);
        window->draw_line(offset.x, offset.y,  arrow_sec.x,  arrow_sec.y, 3);
    }
};
