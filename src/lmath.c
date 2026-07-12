#include "lmath.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void v3_print(v3 v)
{
	printf("[%f, %f, %f]\n", v.x, v.y, v.z);
}

void v4_print(v4 v)
{
	printf("[%f, %f, %f, %f]\n", v.x, v.y, v.z, v.w);
}

v4 v4mk(float x, float y, float z, float w)
{
	return (v4){{ x, y, z, w }};
}

v3 v3mk(float x, float y, float z)
{
	return (v3){{ x, y, z }};
}

v4 v3v4(v3 v)
{
	return v4mk(v.x, v.y, v.z, 1.0f);
}

v3 v4v3(v4 v)
{
	return v3mk(v.x, v.y, v.z);
}

v3 cross_product(v3 v, v3 u)
{
	return (v3) {
		.x = (v.y * u.z) - (v.z * u.y),
		.y = (v.z * u.x) - (v.x * u.z),
		.z = (v.x * u.y) - (v.y * u.x)
	};
}

float v3_magn(v3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

v3 v3_norm(v3 v)
{
	float m = v3_magn(v);
	return v3mk(v.x / m, v.y / m, v.z / m);
}

v3 v3_sum(v3 u, v3 v)
{
	return v3mk(u.x + v.x, u.y + v.y, u.z + v.z);
}

v3 v3_times_scalar(v3 v, float f)
{
	return v3mk(v.x * f, v.y * f, v.z * f);
}

m4 m4_identity()
{
	return (m4) {
		.f00 = 1.0f,
		.f11 = 1.0f,
		.f22 = 1.0f,
		.f33 = 1.0f
	};
}

static int m4idx(int i, int j)
{
	return i * 4 + j;
}

void m4_print(m4 m)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++)
			printf("%f ", m.cmps[m4idx(i, j)]);
		printf("\n");
	}
}

m4 m4_rotation_x(float theta)
{
	return (m4){{
			1.0f, 0.0f,        0.0f,         0.0f,
			0.0f, cosf(theta), -sinf(theta), 0.0f,
			0.0f, sinf(theta), cosf(theta),  0.0f,
			0.0f, 0.0f,        0.0f,         1.0f
		}};
}

m4 m4_rotation_y(float theta)
{
	return (m4){{
			cosf(theta),  0.0f, sinf(theta), 0.0f,
			0.0f,         1.0f, 0.0f,        0.0f,
			-sinf(theta), 0.0f, cosf(theta), 0.0f,
			0.0f,         0.0f, 0.0f,        1.0f
		}};
}

m4 m4_rotation_z(float theta)
{
	return (m4){{
			cosf(theta), -sinf(theta), 0.0f, 0.0f,
			sinf(theta), cosf(theta),  0.0f, 0.0f,
			0.0f,        0.0f,         1.0f, 0.0f,
			0.0f,        0.0f,         0.0f, 1.0f
		}};
}

m4 m4_translation(v3 t)
{
	return (m4){{
			1.0f, 0.0f, 0.0f, t.x,
			0.0f, 1.0f, 0.0f, t.y,
			0.0f, 0.0f, 1.0f, t.z,
			0.0f, 0.0f, 0.0f, 1.0f
		}};
}

v4 m4v4mul(m4 m, v4 v)
{
	v4 mv = {0};
	for (int i = 0; i < 4; i++) {
		float sum = 0.0f;
		for (int j = 0; j < 4; j++)
			sum += m.cmps[m4idx(i, j)] * v.cmps[j];
		mv.cmps[i] = sum;
	}
	return mv;
}

m4 m4mul(m4 a, m4 b)
{
	m4 ab = {0};
	for (int i = 0; i < 4; i++) {
		for (int  j = 0; j < 4; j++) {
			float sum = 0.0f;
			for (int  k = 0; k < 4; k++) {
				int aid = m4idx(i, k);
				int bid = m4idx(k, j);
				sum += a.cmps[aid] * b.cmps[bid];
			}
			ab.cmps[m4idx(i, j)] = sum;
		}
	}
	return ab;
}

m4 m4muls(m4 *ms, int mscnt)
{
	/*
	  multiplication is computed raw major.
	  e.g.
	  mscnt = 4, so:
	  m4 m = ms[4 - 1];
	  m = ms[2] * m;
	  m = ms[1] * m;
	  m = ms[0] * m;
	 */
	assert((mscnt >= 2) && "error: tried to multiply less then 2 matrices");
	m4 m = ms[mscnt - 1];
	for (int i = mscnt - 2; i <= 0; mscnt--) {
		m = m4mul(ms[i], m);
	}
	return m;
}

v3 clip_to_scr(v3 v, int win_w, int win_h)
{
	return v3mk(
		(((v.x + 1.0f) / 2.0f) * win_w),
		((1.0f - ((v.y + 1.0f) / 2.0f)) * win_h),
		v.z
		);
}

v3 project(v4 v)
{
	return v3mk(
		v.x = v.x / v.w,
		v.y = v.y / v.w,
		v.z = v.z / v.w
		);
}

m4 m4_perspective(float near, float far, float ratio, float fov)
{
	m4 p = {0};
	p.f00 = ratio * (1.0f / tanf(fov / 2.0f));
	p.f11 = (1.0f / tanf(fov / 2.0f));
	p.f22 = far / (far - near);
	p.f23 = -far * near / (far - near);
	p.f32 = 1.0f;
	return p;
}

camera camera_mk(v3 pos, v3 dir)
{
	return (camera){ pos, dir };
}

camera camera_init()
{
	return (camera){
		(v3){0}, (v3){ 0.0f, 0.0f, 1.0f },
		.yaw = M_PI / 2.0f,
	};
}

static v3 world_up = { 0.0f, 1.0f, 0.0 };
m4 m4_camera_view(camera c)
{
	/* assert(v3_magn(c.dir) == 1.0f); */
	v3 z = v3_norm(c.dir);
	v3 x = v3_norm(cross_product(world_up, z));
	v3 y = v3_norm(cross_product(z, x));
	m4 view = {
		x.x,  x.y,  x.z,  0.0f,
		y.x,  y.y,  y.z,  0.0f,
		z.x,  z.y,  z.z,  0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	m4 trans = {
		1.0f, 0.0f, 0.0f, -c.pos.x,
		0.0f, 1.0f, 0.0f, -c.pos.y,
		0.0f, 0.0f, 1.0f, -c.pos.z,
		0.0f, 0.0f, 0.0f,  1.0f
	};
	view = m4mul(view, trans);
	return view;
}

float rand_float(void)
{
    return (float)rand() / (float)RAND_MAX;
}


