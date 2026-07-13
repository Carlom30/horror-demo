#include "mesh.h"

#include <assert.h>

#include "render.h"

triangle trimk(v3 p0, v3 p1, v3 p2)
{
	return (triangle){ p0, p1, p2 };
}

static float edge_function(v3 v0, v3 v1, v3 p)
{
	/* just the magnitude of the cross product between (v0 - v1) and (p - v0)
	   or the determinant of the matrix describing the (v0 - v1) and (p - v0) vector space.
	   If this value is negative, then the point p is "on the left" of the edge, meaning is outside
	   of the triangle */
	return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y ) * (v1.x - v0.x);
}

rect find_triangle_box(triangle t)
{
	rect r = {
		.x = MIN(MIN(t.p0.x, t.p1.x), t.p2.x),
		.y = MIN(MIN(t.p0.y, t.p1.y), t.p2.y),
	};
	r.w = MAX(MAX(t.p0.x, t.p1.x), t.p2.x) - r.x;
	r.h = MAX(MAX(t.p0.y, t.p1.y), t.p2.y) - r.y;
	return r;
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

m4 mesh_transform(mesh m)
{
	m4 tr = m4_rotation_y(m.theta);
	tr = m4mul(m4_translation(m.pos), tr);
	return tr;
}

/* https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html */
void raster_triangle(triangle t)
{
	rect r = find_triangle_box(t);
	for (int y = r.y; y < r.y + r.h; y++) {
		for (int x = r.x; x < r.x + r.w; x++) {
			v3 p = v3mk(x, (float)y, 0);
			float w0 = edge_function(t.p0, t.p1, p);
			float w1 = edge_function(t.p1, t.p2, p);
			float w2 = edge_function(t.p2, t.p0, p);
			if (w0 >= 0 && w1 >= 0 && w2 >= 0) {
				/* nice, point is inside the triangle */
				render_set_color(255 * w0, 255 * w1, 255 * w2);
				render_draw_point(x, y);
			}
		}
	}
}
