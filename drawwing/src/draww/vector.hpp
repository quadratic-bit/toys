#pragma once
#include <math.h>

struct Vector {
	float x;
	float y;
};

struct ArrowHead {

	Vector left;
	Vector right;
};

void rotate_vector(Vector &vec, float angle);
