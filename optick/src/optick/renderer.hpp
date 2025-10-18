#pragma once
#include <cassert>
#include <swuix/widgets/handle.hpp>

#include "trace/scene.hpp"
#include "trace/cs.hpp"

static inline Scene make_demo_scene() {
    Scene scn;

    const MaterialOpaque *ground_plane = new MaterialOpaque(/*kd*/0.9);
    //scn.objects.push_back(new Sphere(
    //    Vector3(0, -1004, 0), 1000,
    //    Color(0.9, 0.9, 0.9),
    //    ground_plane
    //));
    scn.objects.push_back(new Plane(
        Vector3(0, -4, 0), Vector3(0, 1, 0),
        Color(0.9, 0.9, 0.9),
        ground_plane
    ));

    const MaterialOpaque *solid = new MaterialOpaque(/*kd*/1.0, /*ks*/1.0, /*shininess*/32);
    scn.objects.push_back(new Sphere(
        Vector3(-1.5, -0.2, -5), 0.7,
        //Color(0.9, 0.1, 0.2),
        Color(1, 0, 0),
        solid
    ));

    const MaterialReflective *mirror = new MaterialReflective();
    scn.objects.push_back(new Sphere(
        Vector3(-1.5, -0.2, -3.5), 0.8,
        Color(0.9, 0.8, 0.7),
        mirror
    ));

    const MaterialReflective *mirror2 = new MaterialReflective();
    scn.objects.push_back(new Sphere(
        Vector3(6, 0, -25), 10,
        //Color(0.3, 0.3, 0.9),
        Color(0, 1, 1),
        mirror2
    ));

    // clear-ish glass sphere
    const MaterialRefractive *glass = new MaterialRefractive(/*ior*/1.5);
    scn.objects.push_back(new Sphere(
        Vector3(0.2, 0.0, -2.5), 0.6,
        Color(0.95, 1.0, 1.0),
        glass
    ));

    // lightly tinted glass (water-like ior)
    const MaterialRefractive *water_glass = new MaterialRefractive(/*ior*/1.33);
    scn.objects.push_back(new Sphere(
        Vector3(1.2, -0.1, -3.0), 0.7,
        Color(0.7, 0.9, 1.0),
        water_glass
    ));

    // Lights
    scn.lights.push_back(PointLight(Vector3(-2, 2.5, -1.5), Color(1.0,0.95,0.9), 60.0)); // warm key
    scn.lights.push_back(PointLight(Vector3( 2, 1.0,  0.5), Color(0.7,0.8,1.0),  25.0)); // cool fill

    return scn;
}

#define N_WORKERS 6

class Renderer : public TitledWidget {
    Texture front_buffer;
    Texture back_buffer;

    Scene       scene;
    Camera      cam;
    CameraBasis cb;
    ViewCS      cs;

    int view_w, view_h;
    std::vector<Color> buf;  // size viewW_*viewH_
    std::vector<unsigned char> pixel_bitmask;

    int  next_x, next_y;
    bool initialized;

    int max_depth;
    double eps;

    SwuixThread workers[N_WORKERS];
    SwuixMutex  job_mtx;
    SwuixCond   job_cv;

    bool job_stop;        // signal threads to exit
    bool job_has_work;    // a frame is active
    int  job_next_row;    // next row to take
    int  job_width, job_height;

    std::vector<unsigned char> row_done;     // per-row flag: 1 when row fully traced
    std::vector<unsigned char> row_uploaded; // per-row flag: 1 when row copied to texture

    static int worker_entry(void *self_void);

    void start_frame_jobs() {
        Window::lock_mutex(job_mtx);
        job_width     = view_w;
        job_height    = view_h;
        job_next_row  = 0;
        job_has_work  = true;

        // reset per-row state
        row_done.assign(view_h, 0u);
        row_uploaded.assign(view_h, 0u);

        // pixels/bits already sized in ensure_init_for_size

        Window::broadcast_condition(job_cv);
        Window::unlock_mutex(job_mtx);
    }

    void stop_workers() {
        Window::lock_mutex(job_mtx);
        job_stop = true;
        Window::broadcast_condition(job_cv);
        Window::unlock_mutex(job_mtx);
        for (int i = 0; i < N_WORKERS; ++i) {
            if (workers[i]) {
                Window::wait_thread(workers[i]);
                workers[i] = 0;
            }
        }
    }

    void fill_texture_with_background(Window *window, Texture *tex) {
        TextureHandle texh;
        if (!tex->lock(&texh)) return;
        for (int iy = 0; iy < view_h; ++iy) {
            uint32_t *pixels = texh.get_row(iy);
            for (int ix = 0; ix < view_w; ++ix) {
                Ray pr = Ray::primary(cam, cb, ix, iy, view_w, view_h);
                Color bg = scene.sample_background(pr.d);
                pixels[ix] = window->map_rgba(Color::encode(bg.r), Color::encode(bg.g), Color::encode(bg.b), 255);
            }
        }
        tex->unlock(&texh);
    }

    /**
     * Lazily (re)initialize buffers and texture if the view size changed
     */
    void ensure_init_for_size(Window *window, int vw, int vh) {
        if (initialized && vw == view_w && vh == view_h) return;

        view_w = vw;
        view_h = vh;

        buf.clear();
        buf.resize(view_w * view_h, Color(0, 0, 0));

        pixel_bitmask.clear();
        pixel_bitmask.resize(view_w * view_h, 0);

        next_x = next_y = 0;
        initialized = true;

        max_depth = 5;
        eps = 1e-4;

        // build camera
        //cam.pos    = Vector3(0, 4,  0);
        cam.pos    = Vector3(0, 2,  2.5);
        cam.target = Vector3(0, 1, -1);
        cam.up     = Vector3(0, 1,  0);
        cam.vfov   = 45.0;
        cam.width  = (double)view_w;
        cam.height = (double)view_h;
        cb         = CameraBasis::make(cam);

        row_done.assign(view_h, 0u);
        row_uploaded.assign(view_h, 0u);

        front_buffer.destroy();
        back_buffer.destroy();

        window->create_texture(&front_buffer, view_w, view_h);
        window->create_texture(&back_buffer,  view_w, view_h);

        fill_texture_with_background(window, &front_buffer);

        start_frame_jobs();
    }

public:
    Renderer(Rect2F rect, Widget *parent_, State *s)
            : Widget(rect, parent_, s), TitledWidget(rect, parent_, s),
            view_w(0), view_h(0), next_x(0), next_y(0),
            initialized(false), max_depth(5), eps(1e-4) {
        scene = make_demo_scene();

        job_mtx = Window::create_mutex();
        job_cv  = Window::create_condition();
        job_stop = false; job_has_work = false;
        job_next_row = 0; job_width = job_height = 0;

        for (int i = 0; i < N_WORKERS; ++i)
            workers[i] = Window::create_thread(Renderer::worker_entry, "rt_worker", this);
    }

    ~Renderer() {
        stop_workers();
        Window::destroy_condition(job_cv);
        Window::destroy_mutex(job_mtx);
        front_buffer.destroy();
        back_buffer.destroy();
    }

    const char *title() const {
        return "Renderer";
    }

    void render(Window *window, float off_x, float off_y) {
        window->clear_rect(frame, off_x, off_y, 125, 12, 125);

        const int viewX = std::floor(frame.x + off_x);
        const int viewY = std::floor(frame.y + off_y);
        const int viewW = std::floor(frame.w);
        const int viewH = std::floor(frame.h);

        // Only size matters for (re)alloc
        ensure_init_for_size(window, viewW, viewH);

        // TODO: figure out if this if-clause can be assumed true as invariant
        if (front_buffer.is_init()) {
            Rect2F dst = frect(viewX, viewY, viewW, viewH);
            window->render_texture(front_buffer, &dst);
        }

        window->outline(frame, off_x, off_y, 2);
    }

    DispatchResult on_idle(DispatcherCtx ctx, const IdleEvent *ev) {
        (void)ctx;
        if (!initialized) return PROPAGATE;

        Time t0 = Window::now();
        double budget = ev->budget_s;
        Time deadline = ev->deadline;
        const double margin = 1e-4;

        TextureHandle texh;
        if (!back_buffer.lock(&texh)) {
            return PROPAGATE;
        }

        // upload as many finished rows as time allows
        for (;;) {
            Time now = Window::now();
            if ((now - t0) >= budget - margin) break;
            if (now >= deadline - margin) break;

            int y_to_upload = -1;

            // find a finished row that is not yet uploaded
            Window::lock_mutex(job_mtx);
            for (int y = 0; y < view_h; ++y) {
                if (row_done[y] && !row_uploaded[y]) {
                    y_to_upload = y;
                    break;
                }
            }
            Window::unlock_mutex(job_mtx);

            if (y_to_upload < 0) break; // nothing ready right now

            // copy that whole row from buf to texture
            const int y = y_to_upload;
            uint32_t *pixels = texh.get_row(y);

            Color *src = &buf[y * view_w];
            for (int x = 0; x < view_w; ++x) {
                const Color &c = src[x];
                pixels[x] = ctx.window->map_rgba(Color::encode(c.r), Color::encode(c.g), Color::encode(c.b), 255);
            }

            // mark uploaded (no need to hold lock long)
            Window::lock_mutex(job_mtx);
            row_uploaded[y] = 1u;
            Window::unlock_mutex(job_mtx);
        }

        back_buffer.unlock(&texh);

        // finished frame?
        bool all_uploaded = true;
        Window::lock_mutex(job_mtx);
        for (int y = 0; y < view_h; ++y) {
            if (!row_uploaded[y]) {
                all_uploaded = false;
                break;
            }
        }
        Window::unlock_mutex(job_mtx);

        if (all_uploaded) {
            // your animation
            scene.objects[1]->center.x += 0.05;

            front_buffer.swap(back_buffer);

            // reset & kick next frame
            next_x = next_y = 0;
            start_frame_jobs();
        }

        return PROPAGATE;
    }
};

int Renderer::worker_entry(void* self_void) {
    Renderer* self = static_cast<Renderer*>(self_void);

    for (;;) {
        // --- take a row ---
        Window::lock_mutex(self->job_mtx);
        while (!self->job_stop &&
               (!self->job_has_work || self->job_next_row >= self->job_height)) {
            Window::wait_condition(self->job_cv, self->job_mtx);
        }
        if (self->job_stop) {
            Window::unlock_mutex(self->job_mtx);
            break;
        }
        // if all rows already taken, loop to wait for next frame
        if (self->job_next_row >= self->job_height) {
            Window::unlock_mutex(self->job_mtx);
            continue;
        }
        int y = self->job_next_row++;
        Window::unlock_mutex(self->job_mtx);

        // --- render the row into buf ---
        for (int x = 0; x < self->job_width; ++x) {
            Ray pr = Ray::primary(self->cam, self->cb, x, y, self->job_width, self->job_height);
            Color c = self->scene.trace(pr, 0, self->max_depth, self->eps);
            self->buf[y * self->job_width + x] = c;
        }

        // --- mark row done (under lock for visibility) ---
        Window::lock_mutex(self->job_mtx);
        self->row_done[y] = 1u;
        // if y was the last one taken and all ≥height, allow main to start next frame
        Window::unlock_mutex(self->job_mtx);
    }

    return 0;
}
