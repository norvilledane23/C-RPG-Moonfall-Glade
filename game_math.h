#pragma once

float distance(float x1, float x2, float y1, float y2) {
	float x, y, z;

	x = x1 - x2;
	y = y1 - y2;
	z = sqrtf(pow(x, 2) + pow(y, 2));

	return z;
}