#pragma once
#include <vector>

#include "objects.hpp"

struct Scene; // forward-declare for TraceContext

struct TraceContext {
    double  eps;
    int     depth;
    int     max_depth;
    Ray     ray;
    Hit     hit;
    Object *target;
    Scene  *scene;

    TraceContext(double eps_, int depth_, int max_depth_, Ray ray_, Hit hit_, Object *target_, Scene *scene_)
        : eps(eps_), depth(depth_), max_depth(max_depth_), ray(ray_), hit(hit_), target(target_), scene(scene_) {}
};

struct Scene {
    std::vector<Object*> objects;
    Color backgroundTop, backgroundBottom;

    Scene() : backgroundTop(0.7, 0.85, 1.0), backgroundBottom(0.05, 0.05, 0.1) {}

    /**
     * Background color (based on Y component of ray's direction d)
     */
    Color sample_background(const Vector3 &d) const {
        double t = 0.5 * (d.y + 1.0); // map y [-1,1] to t [0,1] (d is normalized)
        if (t < 0) t = 0;
        if (t > 1) t = 1;
        return backgroundTop * t + backgroundBottom * (1.0 - t);
    }

    /**
     * Shadow test from point p towards light direction to_light (unit) up to max_dist
     */
    bool occluded(const Vector3 &p, const Vector3 &to_light, double max_dist, double eps) const {
        Ray sray(p + to_light * eps, to_light); // offset origin to avoid self-intersection
        for (size_t i = 0; i < objects.size(); ++i) {
            Hit h;
            if (!objects[i]->intersect(sray, eps, &h)) continue;
            if (h.dist < max_dist) return true; // blocked by something
        }
        return false;
    }

    /**
     * Ignore the target emissive if it is the first hit
     */
    bool occluded_towards(const Vector3 &p, const Vector3 &to_light,
            double max_dist, double eps, const Object *target_light_geom) const {
        Ray sray(p + to_light * eps, to_light);
        const Object* first_obj = NULL;
        Hit h_first;

        double closest = max_dist;
        for (size_t i = 0; i < objects.size(); ++i) {
            Hit h;
            if (!objects[i]->intersect(sray, eps, &h)) continue;
            if (h.dist < closest) {
                closest = h.dist;
                first_obj = objects[i];
                h_first = h;
            }
        }

        if (!first_obj) return false; // nothing in the way
        if (first_obj == target_light_geom) return false; // hit the light
        return true; // some other geometry blocks
    }

    /**
     * Recursively trace a ray and shade based on materials
     */
    inline Color trace(const Ray &ray, int depth, int max_depth, double eps) {
        if (depth > max_depth) return Color(0, 0, 0);

        // find closest hit
        Hit hit = Hit();
        for (size_t i = 0; i < this->objects.size(); ++i) {
            Hit new_hit;
            bool intersects = this->objects[i]->intersect(ray, eps, &new_hit);
            if (intersects && new_hit.dist < hit.dist) {
                hit = new_hit;
                hit.obj_i = i;
            }
        }

        if (hit.obj_i < 0) {
            return this->sample_background(ray.d);
        }

        Object *sp = this->objects[hit.obj_i];

        TraceContext ctx = TraceContext(eps, depth, max_depth, ray, hit, sp, this);
        return sp->mat->sample(ctx);
    }
};
