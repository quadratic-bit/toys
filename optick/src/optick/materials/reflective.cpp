#include "../trace/scene.hpp"

opt::Color MaterialReflective::sample(TraceContext ctx) const {
    Vector3 R = Vector3::reflect(ctx.ray.d, ctx.hit.norm);
    opt::Color inc_clr = ctx.scene->trace(Ray(ctx.hit.pos + R * ctx.eps, R), ctx.depth + 1, ctx.max_depth, ctx.eps);
    // apply tint + faint base
    return ctx.target->color * inc_clr + opt::Color(0.02, 0.02, 0.02);
}
