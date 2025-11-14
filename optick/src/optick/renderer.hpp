#pragma once
#include <SDL3/SDL_mutex.h>
#include <atomic>
#include <memory>
#include <swuix/widgets/titled.hpp>
#include <swuix/state.hpp>

#include "dr4/math/color.hpp"
#include "dr4/texture.hpp"
#include "swuix/common.hpp"
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

    MaterialOpaque *ground_plane = new MaterialOpaque(/*kd*/0.9);
    scn.objects.push_back(new Plane("ground",
        Vector3(0, -4, 0), Vector3(0, 1, 0),
        opt::Color(0.9, 0.9, 0.9),
        ground_plane
    ));

    MaterialOpaque *normal = new MaterialOpaque(/*kd*/1.0, /*ks*/1.0, /*shininess*/32);
    const Vector3 verts[] = { Vector3(-10, -5, -10), Vector3(-8, -2, -10), Vector3(-11, -1, -10) };
    scn.objects.push_back(new Polygon("triangle", std::vector<Vector3>(verts, verts + 3), opt::Color(1.0, 1.0, 1.0), normal));

    MaterialOpaque *solid = new MaterialOpaque(/*kd*/1.0, /*ks*/1.0, /*shininess*/32);
    scn.objects.push_back(new Sphere("red ball",
        Vector3(-1.5, -0.2, -5), 0.7,
        opt::Color(1, 0, 0),
        solid
    ));

    MaterialReflective *mirror = new MaterialReflective();
    scn.objects.push_back(new Sphere("mirror",
        Vector3(-1.5, -0.2, -3.5), 0.8,
        opt::Color(0.9, 0.8, 0.7),
        mirror
    ));

    MaterialReflective *mirror2 = new MaterialReflective();
    scn.objects.push_back(new Sphere("big mirror",
        Vector3(6, 0, -25), 10,
        opt::Color(0, 1, 1),
        mirror2
    ));

    MaterialRefractive *glass = new MaterialRefractive(/*ior*/1.5);
    scn.objects.push_back(new Sphere("clear glass",
        Vector3(0.2, 0.0, -2.5), 0.6,
        opt::Color(0.95, 1.0, 1.0),
        glass
    ));

    MaterialRefractive *water_glass = new MaterialRefractive(/*ior*/1.33);
    scn.objects.push_back(new Sphere("tinted water glass",
        Vector3(1.2, -0.1, -3.0), 0.7,
        opt::Color(0.7, 0.9, 1.0),
        water_glass
    ));

    // Lights
    MaterialEmissive *glowing = new MaterialEmissive(opt::Color(1.0, 1.0, 1.0));
    scn.objects.push_back(new Sphere("glowing 1",
        Vector3(-2, 2.5, -1.5), 0.25,
        opt::Color(1, 1, 1),  // tint
        glowing
    ));
    scn.objects.push_back(new Sphere("glowing 2",
        Vector3(2, 1.0, 0.5), 0.25,
        opt::Color(1, 1, 1),  // tint
        glowing
    ));

    return scn;
}

#define N_WORKERS 4

class Renderer final : public TitledWidget {
    Scene  scene;
    Camera cam;
    ViewCS cs;

    dr4::Image *front_img = nullptr;
    dr4::Image *back_img  = nullptr;

    bool initialized;

    int max_depth;
    double eps;

    SDL_Thread    *workers[N_WORKERS];
    SDL_Mutex     *job_mtx;
    SDL_Condition *job_cv;

    bool job_stop;        // signal threads to exit
    bool job_has_work;    // a frame is active
    int  job_width, job_height;
    int  job_next_tile;  // next index into tile_order

    enum { TILE = 16 };

    int tile_w, tile_h;
    int num_tiles;  // tile_w * tile_h

    std::atomic<bool> tiles_need_present {false};

    std::unique_ptr<std::atomic<uint8_t>[]> tile_done;
    std::vector<uint8_t>                    tile_uploaded;
    std::vector<int>                        tile_order;

    unsigned rng_state;  // simple per-frame RNG for shuffling

    static int workerEntry(void *self_void);

    bool alive = true;

    void buildTiles() {
        tile_w = (back_img->GetWidth() + TILE - 1) / TILE;
        tile_h = (back_img->GetHeight() + TILE - 1) / TILE;

        num_tiles = tile_w * tile_h;
        allocTileFlags(num_tiles);

        tile_order.resize(num_tiles);
        for (int i = 0; i < num_tiles; ++i) tile_order[i] = i;

        rng_state = (unsigned)(state->window->GetTime() * 1e6);
        shuffle(tile_order, rng_state);

        job_next_tile = 0;
    }

    void allocTileFlags(int count) {
        tile_done.reset(new std::atomic<uint8_t>[count]);
        for (int i = 0; i < count; ++i)
            tile_done[i].store(0, std::memory_order_relaxed);

        tile_uploaded.assign(count, 0);
    }

    void startFrameJobs() {
        SDL_LockMutex(job_mtx);

        buildTiles();

        job_width     = back_img->GetWidth();
        job_height    = back_img->GetHeight();
        job_has_work  = true;

        SDL_BroadcastCondition(job_cv);
        SDL_UnlockMutex(job_mtx);
    }

    void stopWorkers() {
        SDL_LockMutex(job_mtx);
        job_stop = true;
        SDL_BroadcastCondition(job_cv);
        SDL_UnlockMutex(job_mtx);
        for (int i = 0; i < N_WORKERS; ++i) {
            if (workers[i]) {
                SDL_WaitThread(workers[i], 0);
                workers[i] = 0;
            }
        }
    }

    void fillBackground(dr4::Image *buf) {
        for (int iy = 0; iy < buf->GetHeight(); ++iy) {
            for (int ix = 0; ix < buf->GetWidth(); ++ix) {
                Ray pr = Ray::primary(cam, ix, iy, buf->GetWidth(), buf->GetHeight());
                opt::Color bg = scene.sampleBackground(pr.d);
                buf->SetPixel(ix, iy,
                    dr4::Color(
                        opt::Color::encode(bg.r),
                        opt::Color::encode(bg.g),
                        opt::Color::encode(bg.b),
                    255)
                );
            }
        }
        texture->Draw(*buf);
    }

    /**
     * Lazily (re)initialize buffers and texture if the view size changed
     */
    void ensureInit(int vw, int vh) {
        if (initialized && vw == front_img->GetWidth() && vh == front_img->GetHeight()) return;

        if (front_img) { delete front_img; front_img = nullptr; }
        if (back_img)  { delete back_img;  back_img  = nullptr; }

        // TODO: check on memory leaks
        front_img = state->window->CreateImage();
        front_img->SetSize({static_cast<float>(vw), static_cast<float>(vh)});
        front_img->SetPos({0, 0});

        back_img = state->window->CreateImage();
        back_img->SetSize({static_cast<float>(vw), static_cast<float>(vh)});
        back_img->SetPos({0, 0});

        if (!texture) texture = state->window->CreateTexture();
        texture->SetSize({static_cast<float>(vw), static_cast<float>(vh)});

        buildTiles();

        // build camera
        cam = Camera(Vector3(0, 2, 2.5), 45.0, vw, vh);
        max_depth = 5;
        eps = 1e-4;

        fillBackground(front_img);
        fillBackground(back_img);
        texture->Draw(*front_img);

        startFrameJobs();
        requestRedraw();
        initialized = true;
    }

public:
    Renderer(Rect2f rect, Widget *p, State *s)
            : Widget(rect, p, s), TitledWidget(rect, p, s),
            cam(Vector3(0, 2, 2.5), 45.0, rect.size.x, rect.size.y),
            initialized(false), max_depth(5), eps(1e-4) {
        scene = makeDemoScene();

        job_mtx      = SDL_CreateMutex();
        job_cv       = SDL_CreateCondition();
        job_stop     = false;
        job_has_work = false;
        job_width    = job_height = 0;

        for (int i = 0; i < N_WORKERS; ++i)
            workers[i] = SDL_CreateThread(Renderer::workerEntry, "rt_worker", this);
    }

    ~Renderer() {
        stopWorkers();
        for (int i = 0; i < N_WORKERS; ++i)
            SDL_DetachThread(workers[i]);

        if (job_cv)  SDL_DestroyCondition(job_cv);
        if (job_mtx) SDL_DestroyMutex(job_mtx);
    }

    const char *title() const override {
        return "Renderer";
    }

    const Scene &getScene() {
        return scene;
    }

    Camera *getCamera() {
        return &cam;
    }

    void drawWireframe(
            const AABB &bbox,
            int view_w, int view_h,
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
            dr4::Line *line = thickLine(
                    state->window,
                    {static_cast<float>(sx[a]),
                     static_cast<float>(sy[a])},
                    {static_cast<float>(sx[c]),
                     static_cast<float>(sy[c])},
            {r, g, b}, 1);
            texture->Draw(*line);
        }
    }

    void draw() override {
        const int viewW = std::floor(frame().size.x);
        const int viewH = std::floor(frame().size.y);
        ensureInit(viewW, viewH);

        texture->Draw(*front_img);

        for (size_t i = 0; i < scene.objects.size(); ++i) {
            const Object *obj = scene.objects[i];
            if (!obj->selected()) continue;
            AABB box;
            if (!obj->worldAABB(&box)) continue;
            drawWireframe(box, viewW, viewH, 255, 80, 0);
        }

        outline(state->window, frame(), 2, {CLR_BORDER});
    }

    void blit(Texture *target, Vec2f acc) override {
        if (tiles_need_present.exchange(false, std::memory_order_acq_rel)) {
            requestRedraw();
        }

        TitledWidget::blit(target, acc);
    }

    void enableRender()  { alive = true; }
    void disableRender() { alive = false; }
    void toggleRender()  { alive ^= true; }

    DispatchResult onIdle(DispatcherCtx, const IdleEvent *ev) override {
        if (!alive) return PROPAGATE;

        const int viewW = int(std::floor(frame().size.x));
        const int viewH = int(std::floor(frame().size.y));
        ensureInit(viewW, viewH);

        bool any_uploaded = false;

        for (;;) {
            int ti = -1;
            for (int i = 0; i < num_tiles; ++i) {
                if (!tile_uploaded[i] &&
                        tile_done[i].load(std::memory_order_acquire) != 0) {
                    ti = i;
                    break;
                }
            }
            if (ti < 0) break; // nothing to do

            const int tx = ti % tile_w, ty = ti / tile_w;
            const int x0 = tx * TILE, y0 = ty * TILE;
            const int x1 = std::min(x0 + TILE, (int)front_img->GetWidth());
            const int y1 = std::min(y0 + TILE, (int)front_img->GetHeight());

            for (int y = y0; y < y1; ++y)
                for (int x = x0; x < x1; ++x)
                    front_img->SetPixel(x, y, back_img->GetPixel(x, y));

            tile_uploaded[ti] = 1;
            any_uploaded = true;

            if (ev && ev->deadline > 0) {
                //  Windownow() in seconds:
                // if (Windownow() >= ev->deadline - margin) break;
            }
        }

        if (any_uploaded) {
            texture->Draw(*front_img);
            requestRedraw();
        }

        bool all_uploaded = true;
        for (uint8_t u : tile_uploaded) { if (!u) { all_uploaded = false; break; } }
        if (all_uploaded) {
            scene.objects[2]->center.x += 0.05f;

            buildTiles();
            startFrameJobs();
            requestRedraw();
        }

        return PROPAGATE;
        return PROPAGATE;
    }
};

inline int Renderer::workerEntry(void *self_void) {
    Renderer *self = static_cast<Renderer*>(self_void);

    for (;;) {
        // take a tile
        SDL_LockMutex(self->job_mtx);
        while (!self->job_stop &&
               (!self->job_has_work || self->job_next_tile >= self->num_tiles)) {
            SDL_WaitCondition(self->job_cv, self->job_mtx);
        }
        if (self->job_stop) {
            SDL_UnlockMutex(self->job_mtx);
            break;
        }
        // if all tiles already taken, loop to wait for next frame
        if (self->job_next_tile >= self->num_tiles) {
            SDL_UnlockMutex(self->job_mtx);
            continue;
        }
        const int ord_idx = self->job_next_tile++;
        const int tile_id = self->tile_order[ord_idx];

        SDL_UnlockMutex(self->job_mtx);

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
                opt::Color c = self->scene.trace(pr, 0, self->max_depth, self->eps);
                self->back_img->SetPixel(x, y, dr4::Color(
                    opt::Color::encode(c.r),
                    opt::Color::encode(c.g),
                    opt::Color::encode(c.b),
                    255
                ));
            }
        }

        self->tile_done[tile_id].store(1, std::memory_order_release);
        self->tiles_need_present.store(true, std::memory_order_release);
    }

    return 0;
}
