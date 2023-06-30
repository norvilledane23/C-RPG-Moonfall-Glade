// Student Name: Norville Amao

#pragma once

typedef struct {
	float x1;
	float x2;
	float y1;
	float y2;
} quadrilateral;

float distance(float x1, float x2, float y1, float y2);
float right_border_check(float coord, float half_width);
float left_border_check(float coord, float half_width);
float bottom_border_check(float coord, float half_height);
float top_border_check(float coord, float half_height);
bool simple_mouse_collision(quadrilateral sprite, float mouse_x, float mouse_y);

float distance(float x1, float x2, float y1, float y2) {
	float x, y, z;

	x = x1 - x2;
	y = y1 - y2;
	z = sqrtf(pow(x, 2) + pow(y, 2));

	return z;
}

// Player coordinates are the center of the sprite. To calculate minimum distance of sprite to map border, corresponding border sprite coordinate is calculated
float right_border_check(float coord, float half_width) {
	return ( (1200 - (coord + half_width) ) < 6 ) ? 1200 - (coord + half_width) : 6;
}

float left_border_check(float coord, float half_width) {
	return ( (coord - half_width) < 6 ) ? coord - half_width : 6;
}

float bottom_border_check(float coord, float half_height) {
	return ( (900 - (coord + half_height)) < 6 ) ? 900 - (coord + half_height) : 6;
}

float top_border_check(float coord, float half_height) {
	return ((coord - half_height) < 6) ? coord - half_height : 6;
}

// Collision check between cursor tip and any sprite (quadrilateral)
bool simple_mouse_collision(quadrilateral sprite, float mouse_x, float mouse_y) {
	return sprite.x1 <= mouse_x && sprite.x2 >= mouse_x && sprite.y1 <= mouse_y && sprite.y2 >= mouse_y;
}