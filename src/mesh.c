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

/*
  WAIT WHY IS THIS FUNCTION TAKING THE PERSPECTIVE MATRIX AS A PARAMETER??
  CALM DOWN! Let me explane, the convenience of the perspective projection matrix
  is the fact that you only calculate it once per frame, so recalculating it for every
  mesh does not make any sense. FOR NOW im passing it to this functio, will probably change it later (?).
*/
void mesh_render(const mesh *m, m4 view, m4 perspective)
{
	int ww, wh;
	render_getwh(&ww, &wh);
	assert(m);
	float theta = m->theta;
	for (int i = 0; i < DA_COUNT(m->tris); i++) {
		triangle tri = m->tris[i];
		v4 p0 = v3v4(tri.p0);
		v4 p1 = v3v4(tri.p1);
		v4 p2 = v3v4(tri.p2);
		m4 tr = m4_rotation_y(theta);
		tr = m4mul(m4_rotation_x(theta), tr);
		tr = m4mul(m4_rotation_z(theta), tr);
		tr = m4mul(m4_translation(m->pos), tr);
		tr = m4mul(view, tr);
		tr = m4mul(perspective, tr);
		p0 = m4v4mul(tr, p0);
		p1 = m4v4mul(tr, p1);
		p2 = m4v4mul(tr, p2);
		if (p0.w <= 0.0f || p1.w <= 0.0f || p2.w <= 0.0f) continue;
		v3 pp0 = (clip_to_scr(project(p0), ww, wh));
		v3 pp1 = (clip_to_scr(project(p1), ww, wh));
		v3 pp2 = (clip_to_scr(project(p2), ww, wh));
		render_set_color(0, 200, 0);
		render_plot_line(pp0.x, pp0.y, pp1.x, pp1.y);
		render_plot_line(pp1.x, pp1.y, pp2.x, pp2.y);
		render_plot_line(pp2.x, pp2.y, pp0.x, pp0.y);
		triangle ptri = trimk(pp0, pp1, pp2);
		rect pr = find_triangle_box(ptri);
		render_set_color(200, 0, 0);
		/* render_draw_rect(&pr); */
	}
}
