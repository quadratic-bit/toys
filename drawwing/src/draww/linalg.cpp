#include <SDL3_gfx/SDL3_gfxPrimitives.h>

#include "linalg.hpp"
#include "window.hpp"

static const double ARROW_HEAD_SIZE = 0.5;

Vector3 operator*(double left, const Vector3 &right) {
	return right * left;
}

Vector2 operator*(double left, const Vector2 &right) {
	return right * left;
}

void DrawWindow::blit_vector(const CoordinateSystem * const cs, const Vector2 * const vec) {
	float screen_x = cs->x_space_to_screen(vec->x);
	float screen_y = cs->y_space_to_screen(vec->y);

	Vector2 back_dir = -(!*vec);
	Vector2 right_wing = ((!*vec).perp_right() + back_dir) * ARROW_HEAD_SIZE + *vec;
	Vector2 left_wing = ((!*vec).perp_left() + back_dir) * ARROW_HEAD_SIZE + *vec;

	cs->vector2_space_to_screen(&right_wing);
	cs->vector2_space_to_screen(&left_wing);

	// Vector
	thickLineRGBA(renderer, cs->y_axis.center, cs->x_axis.center, screen_x, screen_y, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);

	// Head
	thickLineRGBA(renderer, right_wing.x, right_wing.y, screen_x, screen_y, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
	thickLineRGBA(renderer, left_wing.x, left_wing.y, screen_x, screen_y, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
}
