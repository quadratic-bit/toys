#include "../trace/scene.hpp"

Color MaterialEmissive::sample(TraceContext ctx) const {
    return ctx.target->color * Le;
}
