#include "mesh.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "loop.h"
#include "render.h"

triangle trimk(v3 p0, v3 p1, v3 p2)
{
	return (triangle){ p0, p1, p2 };
}

rect find_triangle_box(triangle t)
{
	rect r = {
		.x = floorf(MIN(MIN(t.p0.x, t.p1.x), t.p2.x)),
		.y = floorf(MIN(MIN(t.p0.y, t.p1.y), t.p2.y)),
	};
	r.w = ceilf(MAX(MAX(t.p0.x, t.p1.x), t.p2.x)) - r.x;
	r.h = ceilf(MAX(MAX(t.p0.y, t.p1.y), t.p2.y)) - r.y;
	return r;
}

m4 mesh_transform(mesh m)
{
	m4 tr = m4_rotation_y(m.theta);
	tr = m4mul(m4_rotation_y(m.theta), tr);
	tr = m4mul(m4_translation(m.pos), tr);
	return tr;
}

mesh mesh_init()
{
	mesh m = {0};
	DA_ALLOC(m.tris);
	return m;
}

mesh mesh_cube(const v3 *pos)
{
	v3 ps[] = {
		(v3){{  0.5f,  0.5f,  0.5f }},
		(v3){{  0.5f, -0.5f,  0.5f }},
		(v3){{ -0.5f, -0.5f,  0.5f }},
		(v3){{ -0.5f,  0.5f,  0.5f }},
		(v3){{  0.5f,  0.5f, -0.5f }},
		(v3){{  0.5f, -0.5f, -0.5f }},
		(v3){{ -0.5f, -0.5f, -0.5f }},
		(v3){{ -0.5f,  0.5f, -0.5f }}
	};
	mesh c = {0};
	DA_ALLOC(c.tris);
	DA_APPEND(c.tris, trimk(ps[0], ps[1], ps[2]));
	DA_APPEND(c.tris, trimk(ps[0], ps[2], ps[3]));
	DA_APPEND(c.tris, trimk(ps[5], ps[4], ps[7]));
	DA_APPEND(c.tris, trimk(ps[5], ps[7], ps[6]));
	DA_APPEND(c.tris, trimk(ps[4], ps[5], ps[1]));
	DA_APPEND(c.tris, trimk(ps[4], ps[1], ps[0]));
	DA_APPEND(c.tris, trimk(ps[3], ps[2], ps[6]));
	DA_APPEND(c.tris, trimk(ps[3], ps[6], ps[7]));
	DA_APPEND(c.tris, trimk(ps[4], ps[0], ps[3]));
	DA_APPEND(c.tris, trimk(ps[4], ps[3], ps[7]));
	DA_APPEND(c.tris, trimk(ps[1], ps[5], ps[6]));
	DA_APPEND(c.tris, trimk(ps[1], ps[6], ps[2]));
	if (pos)
		c.pos = *pos;
	return c;
}

v3 triangle_normal(triangle t)
{
	return v3_norm(cross_product(v3_sub(t.p1, t.p0), v3_sub(t.p2, t.p0)));
}

float triangle_area(triangle t)
{
	return 0.5f * ((t.p1.x - t.p0.x) * (t.p2.y - t.p0.y) - (t.p2.x - t.p0.x) * (t.p1.y - t.p0.y));
}

