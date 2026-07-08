#pragma once

#include "lmath.h"
#include "utils.h"

typedef struct {
	union {
		v3 ps[3];
		struct {
			v3 p0;
			v3 p1;
			v3 p2;
		};
	};
	v3 norm;
} triangle;

typedef struct {
	DA(triangle) tris;
	v3 pos;
} mesh;

triangle trimk(v3, v3, v3);
mesh mesh_cube(const v3 *pos);
