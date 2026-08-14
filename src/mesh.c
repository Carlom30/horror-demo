#include "mesh.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "render.h"
#include "obj.h"

static DA(mesh) loaded_meshes;

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

void mesh_load_all(void)
{
	/* this will, for now, load evey mesh into an array.
	   But i have questions:
	   0) should this be an hashmap?
	   1) should i really load evey mesh into the memory? */
	DA_ALLOC(loaded_meshes);
	mesh m = {0};
	obj_load_mesh("assets/cube.obj", &m);
	DA_APPEND(loaded_meshes, m);
	obj_load_mesh("assets/icosphere.obj", &m);
	DA_APPEND(loaded_meshes, m);
	obj_load_mesh("assets/cow.obj", &m);
	DA_APPEND(loaded_meshes, m);
	obj_load_mesh("assets/al.obj", &m);
	DA_APPEND(loaded_meshes, m);
	obj_load_mesh("assets/peng/PenguinBaseMesh.obj", &m);
	image_load("assets/peng/PenguinText.png", &m.texture);
	DA_APPEND(loaded_meshes, m);
	obj_load_mesh("assets/skull/skull.obj", &m);
	image_load("assets/skull/skull.jpg", &m.texture);
	DA_APPEND(loaded_meshes, m);
	/* TODO make material */
	for (int i = 0; i < DA_COUNT(loaded_meshes); i++) {
		mesh *m = &loaded_meshes[i];
		for (int j = 0; j < DA_COUNT(m->tris); j++) {
			m->tris[j].p0col = v3mk(255, 255, 255);
			m->tris[j].p1col = v3mk(255, 255, 255);
			m->tris[j].p2col = v3mk(255, 255, 255);
		}
	}
}

mesh mesh_get_by_name(enum mesh_name name)
{
	return loaded_meshes[name];
}

mesh mesh_alloc()
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

