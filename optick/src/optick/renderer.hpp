#pragma once
#include <cstdio>

#include <swuix/widgets/handle.hpp>

#include "trace/scene.hpp"
#include "trace/cs.hpp"

static inline Scene make_demo_scene() {
    Scene scn;

    const MaterialOpaque *ground_plane = new MaterialOpaque(/*kd*/0.9);
    scn.spheres.push_back(Sphere(
                Vector3(0, -1004, 0), 1000,
                Color(0.9, 0.9, 0.9),
                ground_plane
                ));

    const MaterialOpaque *solid = new MaterialOpaque(/*kd*/1.0, /*ks*/1.0, /*shininess*/32);
    scn.spheres.push_back(Sphere(
                Vector3(-1.5, -0.2, -5), 0.7,
                Color(0.9, 0.8, 0.7),
                solid
                ));

    const MaterialReflective *mirror = new MaterialReflective();
    scn.spheres.push_back(Sphere(
                Vector3(-1.5, -0.2, -3.5), 0.8,
                Color(0.9, 0.8, 0.7),
                mirror
                ));

    // clear-ish glass sphere
    const MaterialRefractive *glass = new MaterialRefractive(/*ior*/1.5);
    scn.spheres.push_back(Sphere(
                Vector3(0.2, 0.0, -2.5), 0.6,
                Color(0.95, 1.0, 1.0),
                glass
                ));

    // lightly tinted glass (water-like ior)
    const MaterialRefractive *water_glass = new MaterialRefractive(/*ior*/1.33);
    scn.spheres.push_back(Sphere(
                Vector3(1.2, -0.1, -3.0), 0.7,
                Color(0.7, 0.9, 1.0),
                water_glass
                ));

    // Lights
    scn.lights.push_back(PointLight(Vector3(-2, 2.5, -1.5), Color(1.0,0.95,0.9), 60.0)); // warm key
    scn.lights.push_back(PointLight(Vector3( 2, 1.0,  0.5), Color(0.7,0.8,1.0),  25.0)); // cool fill

    return scn;
}

class Renderer : public TitledWidget {
    SwuixTexture front_buffer;
    SwuixTexture back_buffer;

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

        // Build camera from size (not position)
        cam.pos    = Vector3(0, 2,  2.5);
        cam.target = Vector3(0, 1, -1);
        cam.up     = Vector3(0, 1,  0);
        cam.vfov   = 45.0;
        cam.width  = (double)view_w;
        cam.height = (double)view_h;
        cb         = CameraBasis::make(cam);


        window->destroy_texture(&front_buffer);
        window->destroy_texture(&back_buffer);

        front_buffer = window->create_texture(view_w, view_h);
        back_buffer  = window->create_texture(view_w, view_h);

        void *tex_pixels = 0; int tex_pitch = 0;
        if (SDL_LockTexture(front_buffer, NULL, &tex_pixels, &tex_pitch)) {
            for (int iy = 0; iy < view_h; ++iy) {
                for (int ix = 0; ix < view_w; ++ix) {
                    Ray pr = Ray::primary(cam, cb, ix, iy, view_w, view_h);
                    Color bg = scene.sample_background(pr.d);
                    // FIXME: cast-align
                    Uint32 *row = (Uint32*)(void*)((Uint8*)tex_pixels + iy * tex_pitch);
                    row[ix] = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32),
                            NULL, Color::encode(bg.r), Color::encode(bg.g), Color::encode(bg.b), 255);
                }
            }
            SDL_UnlockTexture(front_buffer);
        }
    }

public:
    Renderer(Rect2F rect, Widget *parent_, State *s)
        : Widget(rect, parent_, s), TitledWidget(rect, parent_, s),
        front_buffer(NULL), view_w(0), view_h(0), next_x(0), next_y(0),
        initialized(false), max_depth(5), eps(1e-4) {
            scene = make_demo_scene();
        }

    ~Renderer() {
        if (front_buffer) {
            SDL_DestroyTexture(front_buffer);
            front_buffer = NULL;
        }
        if (back_buffer) {
            SDL_DestroyTexture(back_buffer);
            back_buffer = NULL;
        }
    }

    const char *title() const {
        return "Renderer";
    }

    void render(Window *window, float off_x, float off_y) {
        window->clear_rect(frame, off_x, off_y, 125, 12, 125);

        const int viewX = (int)std::floor(frame.x + off_x);
        const int viewY = (int)std::floor(frame.y + off_y);
        const int viewW = (int)std::floor(frame.w);
        const int viewH = (int)std::floor(frame.h);

        // Only size matters for (re)alloc
        ensure_init_for_size(window, viewW, viewH);

        if (front_buffer) {
            SDL_FRect dst = { (float)viewX, (float)viewY, (float)viewW, (float)viewH };
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

        void *tex_pixels = 0; int tex_pitch = 0;
        if (!SDL_LockTexture(back_buffer, NULL, &tex_pixels, &tex_pitch)) return PROPAGATE;

        // until run out of time or finish the image
        for (;;) {
            // budget management
            Time now = Window::now();
            if ((now - t0) >= budget - margin) break;
            if (now >= deadline - margin) break;

            // check for finished
            if (next_y >= view_h) {
                scene.spheres[1].center.x += 0.05;

                SDL_UnlockTexture(back_buffer);

                std::swap(front_buffer, back_buffer);

                next_x = next_y = 0;

                return PROPAGATE;
            }

            // tracing
            int ix = next_x, iy = next_y;
            Ray pr = Ray::primary(cam, cb, ix, iy, view_w, view_h);

            // NOTE: this is _the_ expensive call
            Color c = scene.trace(pr, 0, max_depth, eps);

            int idx = iy * view_w + ix;
            buf[idx] = c;
            pixel_bitmask[idx] = 1;

            // FIXME: cast-align
            Uint32 *row = (Uint32*)(void*)((Uint8*)tex_pixels + iy * tex_pitch);
            row[ix] = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), NULL,
                    Color::encode(c.r), Color::encode(c.g), Color::encode(c.b), 255);

            // scanline order
            ++next_x;
            if (next_x >= view_w) {
                next_x = 0;
                ++next_y;
            }
        }

        SDL_UnlockTexture(back_buffer);

        return PROPAGATE;
    }
};
