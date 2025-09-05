#include <math.h>
#include <SDL3_gfx/SDL3_gfxPrimitives.h>
#include "vector.hpp"
#include "window.hpp"

void rotate_vector(Vector &vec, float angle) {
	// (cos phi, -sin phi) / (sin phi, cos phi)
	float new_x = vec.x * cos(angle) - vec.y * sin(angle);
	float new_y = vec.x * sin(angle) + vec.y * cos(angle);
	vec.x = new_x;
	vec.y = new_y;
}

static ArrowHead arrow_head_45(const Vector& vec, float size) {
	ArrowHead head = { { vec.x, vec.y }, { vec.x, vec.y } };
	float len = std::sqrt(vec.x * vec.x + vec.y * vec.y);
	if (len == 0.0f) return head;  // degenerate

	float ux = vec.x / len, uy = vec.y / len;
	float px = -uy, py = ux;

	const float scale = size * (float)(M_SQRT1_2);  // 45deg

	head.left.x = vec.x - scale * (ux + px);
	head.left.y = vec.y - scale * (uy + py);

	head.right.x = vec.x - scale * (ux - px);
	head.right.y = vec.y - scale * (uy - py);

	return head;
}

void DrawWindow::draw_vector(const CoordinateSystem &cs, Vector &vec) {
	float screen_x = cs.y_axis.center + vec.x * (float)cs.y_axis.scale;
	float screen_y = cs.x_axis.center - vec.y * (float)cs.x_axis.scale;
	ArrowHead head = arrow_head_45(vec, 0.5);

	float head_left_x = cs.y_axis.center + head.left.x * (float)cs.y_axis.scale;
	float head_left_y = cs.x_axis.center - head.left.y * (float)cs.x_axis.scale;
	float head_right_x = cs.y_axis.center + head.right.x * (float)cs.y_axis.scale;
	float head_right_y = cs.x_axis.center - head.right.y * (float)cs.x_axis.scale;

	thickLineRGBA(renderer, cs.y_axis.center, cs.x_axis.center, screen_x, screen_y, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
	thickLineRGBA(renderer, head_left_x, head_left_y, screen_x, screen_y, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
	thickLineRGBA(renderer, head_right_x, head_right_y, screen_x, screen_y, 3, CLR_BLACK, SDL_ALPHA_OPAQUE);
}
