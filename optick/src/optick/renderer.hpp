#pragma once
#include <swuix/widgets/titled.hpp>

#include "trace/camera.hpp"
#include "trace/scene.hpp"
#include "trace/cs.hpp"

static inline unsigned lcg(unsigned &s) {
    s = 1664525u * s + 1013904223u;
    return s;
}

// Fisher–Yates shuffle
static inline void shuffle(std::vector<int> &a, unsigned &state) {
    for (int i = (int)a.size() - 1; i > 0; --i) {
        unsigned r = lcg(state);
        int j = (int)(r % (unsigned)(i + 1));
        int tmp = a[i]; a[i] = a[j]; a[j] = tmp;
    }
}

static inline Scene makeDemoScene() {
    Scene scn;

    const MaterialOpaque *ground_plane = new MaterialOpaque(/*kd*/0.9);
    scn.objects.push_back(new Plane("ground",
        Vector3(0, -4, 0), Vector3(0, 1, 0),
        Color(0.9, 0.9, 0.9),
        ground_plane
    ));

    const MaterialOpaque *normal = new MaterialOpaque(/*kd*/1.0, /*ks*/1.0, /*shininess*/32);
    const Vector3 verts[] = { Vector3(-10, -5, -10), Vector3(-8, -2, -10), Vector3(-11, -1, -10) };
    scn.objects.push_back(new Polygon("triangle", std::vector<Vector3>(verts, verts + 3), Color(1.0, 1.0, 1.0), normal));

    const MaterialOpaque *solid = new MaterialOpaque(/*kd*/1.0, /*ks*/1.0, /*shininess*/32);
    scn.objects.push_back(new Sphere("red ball",
        Vector3(-1.5, -0.2, -5), 0.7,
        Color(1, 0, 0),
        solid
    ));

    const MaterialReflective *mirror = new MaterialReflective();
    scn.objects.push_back(new Sphere("mirror",
        Vector3(-1.5, -0.2, -3.5), 0.8,
        Color(0.9, 0.8, 0.7),
        mirror
    ));

    const MaterialReflective *mirror2 = new MaterialReflective();
    scn.objects.push_back(new Sphere("big mirror",
        Vector3(6, 0, -25), 10,
        Color(0, 1, 1),
        mirror2
    ));

    const MaterialRefractive *glass = new MaterialRefractive(/*ior*/1.5);
    scn.objects.push_back(new Sphere("clear glass",
        Vector3(0.2, 0.0, -2.5), 0.6,
        Color(0.95, 1.0, 1.0),
        glass
    ));

    const MaterialRefractive *water_glass = new MaterialRefractive(/*ior*/1.33);
    scn.objects.push_back(new Sphere("tinted water glass",
        Vector3(1.2, -0.1, -3.0), 0.7,
        Color(0.7, 0.9, 1.0),
        water_glass
    ));

    // Lights
    const MaterialEmissive *glowing = new MaterialEmissive(Color(1.0, 1.0, 1.0));
    scn.objects.push_back(new Sphere("glowing 1",
        Vector3(-2, 2.5, -1.5), 0.25,
        Color(1, 1, 1),  // tint
        glowing
    ));
    scn.objects.push_back(new Sphere("glowing 2",
        Vector3(2, 1.0, 0.5), 0.25,
        Color(1, 1, 1),  // tint
        glowing
    ));

    return scn;
}

#define N_WORKERS 4

class Renderer : public TitledWidget {
    Texture front_buffer;

    Scene       scene;
    Camera      cam;
    ViewCS      cs;

    int view_w, view_h;
    std::vector<Color> buf;  // size view_w * view_h
    std::vector<unsigned char> pixel_bitmask;

    bool initialized;

    int max_depth;
    double eps;

    SwuixThread workers[N_WORKERS];
    SwuixMutex  job_mtx;
    SwuixCond   job_cv;

    bool job_stop;        // signal threads to exit
    bool job_has_work;    // a frame is active
    int  job_width, job_height;
    int  job_next_tile;  // next index into tile_order

    enum { TILE = 16 };

    int tile_w, tile_h;
    int num_tiles;  // tile_w * tile_h

    std::vector<unsigned char> tile_done;
    std::vector<unsigned char> tile_uploaded;
    std::vector<int>           tile_order;

    unsigned rng_state;  // simple per-frame RNG for shuffling

    static int workerEntry(void *self_void);

    void buildTiles() {
        tile_w = (view_w + TILE - 1) / TILE;
        tile_h = (view_h + TILE - 1) / TILE;
        num_tiles = tile_w * tile_h;

        tile_done.assign(num_tiles, 0u);
        tile_uploaded.assign(num_tiles, 0u);

        tile_order.resize(num_tiles);
        for (int i = 0; i < num_tiles; ++i) tile_order[i] = i;

        rng_state = (unsigned)(Window::now() * 1e6);
        shuffle(tile_order, rng_state);

        job_next_tile = 0;
    }

    void startFrameJobs() {
        Window::lock_mutex(job_mtx);

        buildTiles();

        job_width     = view_w;
        job_height    = view_h;
        job_has_work  = true;

        Window::broadcast_condition(job_cv);
        Window::unlock_mutex(job_mtx);
    }

    void stopWorkers() {
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

    void fillTexture(Window *window, Texture *tex) {
        TextureHandle texh;
        if (!tex->lock(&texh)) return;
        for (int iy = 0; iy < view_h; ++iy) {
            uint32_t *pixels = texh.get_row(iy);
            for (int ix = 0; ix < view_w; ++ix) {
                Ray pr = Ray::primary(cam, ix, iy, view_w, view_h);
                Color bg = scene.sampleBackground(pr.d);
                pixels[ix] = window->map_rgba(RGBu8(Color::encode(bg.r), Color::encode(bg.g), Color::encode(bg.b)), 255);
            }
        }
        tex->unlock(&texh);
    }

    /**
     * Lazily (re)initialize buffers and texture if the view size changed
     */
    void ensureInit(Window *window, int vw, int vh) {
        if (initialized && vw == view_w && vh == view_h) return;

        view_w = vw;
        view_h = vh;

        buf.clear();
        buf.resize(view_w * view_h, Color(0, 0, 0));

        pixel_bitmask.clear();
        pixel_bitmask.resize(view_w * view_h, 0);

        initialized = true;

        max_depth = 5;
        eps = 1e-4;

        // build camera
        cam = Camera(Vector3(0, 2, 2.5), 45.0, view_w, view_h);

        front_buffer.destroy();

        window->create_texture(&front_buffer, view_w, view_h);

        fillTexture(window, &front_buffer);

        startFrameJobs();
    }

public:
    Renderer(Rect2F rect, Widget *parent_, State *s)
            : Widget(rect, parent_, s), TitledWidget(rect, parent_, s),
            cam(Vector3(0, 2, 2.5), 45.0, rect.w, rect.h),
            view_w(0), view_h(0), initialized(false), max_depth(5), eps(1e-4) {
        scene = makeDemoScene();

        job_mtx      = Window::create_mutex();
        job_cv       = Window::create_condition();
        job_stop     = false;
        job_has_work = false;
        job_width    = job_height = 0;

        for (int i = 0; i < N_WORKERS; ++i)
            workers[i] = Window::create_thread(Renderer::workerEntry, "rt_worker", this);
    }

    ~Renderer() {
        stopWorkers();
        for (int i = 0; i < N_WORKERS; ++i)
            Window::detach_thread(workers[i]);
        Window::destroy_condition(job_cv);
        Window::destroy_mutex(job_mtx);
        front_buffer.destroy();
    }

    const char *title() const {
        return "Renderer";
    }

    const Scene &getScene() {
        return scene;
    }

    Camera *getCamera() {
        return &cam;
    }

    static void drawWireframe(
            Window *window,
            const AABB &bbox,
            const Camera &cam,
            int view_x, int view_y, int view_w, int view_h, double eps,
            uint8_t r, uint8_t g, uint8_t b
    ) {
        Vector3 v[8] = {
            Vector3(bbox.mn.x, bbox.mn.y, bbox.mn.z), Vector3(bbox.mx.x, bbox.mn.y, bbox.mn.z),
            Vector3(bbox.mx.x, bbox.mx.y, bbox.mn.z), Vector3(bbox.mn.x, bbox.mx.y, bbox.mn.z),
            Vector3(bbox.mn.x, bbox.mn.y, bbox.mx.z), Vector3(bbox.mx.x, bbox.mn.y, bbox.mx.z),
            Vector3(bbox.mx.x, bbox.mx.y, bbox.mx.z), Vector3(bbox.mn.x, bbox.mx.y, bbox.mx.z),
        };
        const int E[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};

        int sx[8], sy[8];
        bool ok[8];

        for (int i = 0; i < 8; ++i) {
            ok[i] = cam.projectPoint(view_w, view_h, v[i], eps, &sx[i], &sy[i]);
        }

        for (int e = 0; e < 12; ++e) {
            int a = E[e][0];
            int c = E[e][1];
            if (!ok[a] || !ok[c]) continue;
            window->draw_line_rgb(view_x + sx[a], view_y + sy[a], view_x + sx[c], view_y + sy[c], 1, RGBu8(r, g, b));
        }
    }

    void render(Window *window, float off_x, float off_y) {
        window->clear_rect(frame, off_x, off_y, RGBu8(125, 12, 125));

        const int viewX = std::floor(frame.x + off_x);
        const int viewY = std::floor(frame.y + off_y);
        const int viewW = std::floor(frame.w);
        const int viewH = std::floor(frame.h);

        // Only size matters for (re)alloc
        ensureInit(window, viewW, viewH);

        // TODO: figure out if this if-clause can be assumed true as invariant
        if (front_buffer.is_init()) {
            Rect2F dst = frect(viewX, viewY, viewW, viewH);
            window->render_texture(front_buffer, &dst);
        }

        for (size_t i = 0; i < scene.objects.size(); ++i) {
            const Object *obj = scene.objects[i];

            if (!obj->selected()) continue;

            AABB box;
            if (!obj->worldAABB(&box)) continue;

            drawWireframe(window, box, cam, viewX, viewY, viewW, viewH, eps, 255, 80, 0);
        }

        window->outline(frame, off_x, off_y, RGB(CLR_BORDER), 2);
    }

    DispatchResult on_idle(DispatcherCtx ctx, const IdleEvent *ev) {
        (void)ctx;
        if (!initialized) return PROPAGATE;

        Time t0 = Window::now();
        double budget = ev->budget_s;
        Time deadline = ev->deadline;
        const double margin = 1e-4;

        TextureHandle texh;
        if (!front_buffer.lock(&texh)) {
            return PROPAGATE;
        }

        // upload as many finished rows as time allows
        for (;;) {
            Time now = Window::now();
            if ((now - t0) >= budget - margin) break;
            if (now >= deadline - margin) break;

            int tile_to_upload = -1;

            // find a finished tile that is not yet uploaded
            Window::lock_mutex(job_mtx);
            for (int ti = 0; ti < num_tiles; ++ti) {
                if (tile_done[ti] && !tile_uploaded[ti]) {
                    tile_to_upload = ti;
                    break;
                }
            }
            Window::unlock_mutex(job_mtx);

            if (tile_to_upload < 0) break;  // nothing ready

            const int tx = tile_to_upload % tile_w;
            const int ty = tile_to_upload / tile_w;
            const int x0 = tx * TILE;
            const int y0 = ty * TILE;
            const int x1 = std::min(x0 + TILE, view_w);
            const int y1 = std::min(y0 + TILE, view_h);

            for (int y = y0; y < y1; ++y) {
                uint32_t *pixels = texh.get_row(y);
                Color    *src    = &buf[y * view_w];
                for (int x = x0; x < x1; ++x) {
                    const Color& c = src[x];
                    pixels[x] = ctx.window->map_rgba(
                        RGBu8(
                            Color::encode(c.r),
                            Color::encode(c.g),
                            Color::encode(c.b)
                        ), 255);
                }
            }

            // mark uploaded (no need to hold lock long)
            Window::lock_mutex(job_mtx);
            tile_uploaded[tile_to_upload] = 1u;
            Window::unlock_mutex(job_mtx);
        }

        front_buffer.unlock(&texh);

        // finished frame?
        bool all_uploaded = true;
        Window::lock_mutex(job_mtx);
        for (int ti = 0; ti < num_tiles; ++ti) {
            if (!tile_uploaded[ti]) {
                all_uploaded = false;
                break;
            }
        }
        Window::unlock_mutex(job_mtx);

        if (all_uploaded) {
            scene.objects[2]->center.x += 0.05;

            startFrameJobs();
        }

        return PROPAGATE;
    }
};

int Renderer::workerEntry(void *self_void) {
    Renderer* self = static_cast<Renderer*>(self_void);

    for (;;) {
        // take a tile
        Window::lock_mutex(self->job_mtx);
        while (!self->job_stop &&
               (!self->job_has_work || self->job_next_tile >= self->num_tiles)) {
            Window::wait_condition(self->job_cv, self->job_mtx);
        }
        if (self->job_stop) {
            Window::unlock_mutex(self->job_mtx);
            break;
        }
        // if all tiles already taken, loop to wait for next frame
        if (self->job_next_tile >= self->num_tiles) {
            Window::unlock_mutex(self->job_mtx);
            continue;
        }
        const int ord_idx = self->job_next_tile++;
        const int tile_id = self->tile_order[ord_idx];

        Window::unlock_mutex(self->job_mtx);

        // tile coords
        const int tx = tile_id % self->tile_w;
        const int ty = tile_id / self->tile_w;
        const int x0 = tx * TILE;
        const int y0 = ty * TILE;
        const int x1 = std::min(x0 + TILE, self->job_width);
        const int y1 = std::min(y0 + TILE, self->job_height);

        // render the tile into buf
        for (int y = y0; y < y1; ++y) {
            for (int x = x0; x < x1; ++x) {
                Ray pr = Ray::primary(self->cam, x, y, self->job_width, self->job_height);
                Color c = self->scene.trace(pr, 0, self->max_depth, self->eps);
                self->buf[y * self->job_width + x] = c;
            }
        }

        // publish completion
        Window::lock_mutex(self->job_mtx);
        self->tile_done[tile_id] = 1u;
        Window::unlock_mutex(self->job_mtx);
    }

    return 0;
}
