#pragma once

#include "lmath.h"
#include "utils.h"
#include "render.h"

typedef struct {
	union {
		v3 ps[3];
		struct {
			v3 p0;
			v3 p1;
			v3 p2;
		};
	};
	union {
		v3 uvs[3];
		struct {
			v3 uv0;
			v3 uv1;
			v3 uv2;
		};
	};
	v3 norm;
	v3 p0col;
	v3 p1col;
	v3 p2col;
} triangle;

typedef struct {
	DA(triangle) tris;
	float theta; /* TODO implement axis rotation */
	v3 pos;
	v3 scale;
	png texture; /* material? */
} mesh;

enum mesh_name {
	MN_CUBE      = 0,
	MN_ICOSPHERE = 1,
	MN_COW       = 2,
	MN_AL        = 3,
	MN_PENG      = 4,
	MN_SKULL     = 5,
	MN_CNT
};

triangle trimk(v3, v3, v3);
rect find_triangle_box(triangle t);
mesh mesh_alloc(void);
void mesh_load_all(void);
mesh mesh_get_by_name(enum mesh_name name);
mesh mesh_cube(const v3 *pos);
v3 triangle_normal(triangle t);
float triangle_area(triangle t);
m4 mesh_transform(mesh m);

