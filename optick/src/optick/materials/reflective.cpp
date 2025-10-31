#include "../trace/scene.hpp"

Color MaterialReflective::sample(TraceContext ctx) const {
    Vector3 R = Vector3::reflect(ctx.ray.d, ctx.hit.norm);
    Color inc_clr = ctx.scene->trace(Ray(ctx.hit.pos + R * ctx.eps, R), ctx.depth + 1, ctx.max_depth, ctx.eps);
    // apply tint + faint base
    return ctx.target->color * inc_clr + Color(0.02, 0.02, 0.02);
}
