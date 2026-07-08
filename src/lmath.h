#pragma once

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define CLAMP(v, min, max) (MAX(MIN(v, max), min))

typedef union {
	float cmps[3];
	struct {
		float x;
		float y;
		float z;
	};
} v3;

typedef union {
	float cmps[4];
	struct {
		float x;
		float y;
		float z;
		float w;
	};
} v4;

typedef union {
	float cmps[4 * 4];
	struct {
		float f00, f01, f02, f03;
		float f10, f11, f12, f13;
		float f20, f21, f22, f23;
		float f30, f31, f32, f33;
	};
} m4;

v4 v4mk(float, float, float, float);
v3 v3mk(float, float, float);
v4 v3v4(v3 v);
v3 v4v3(v4 v);
void v3_print(v3);
void v4_print(v4);

/* matrix */
m4 m4_identity();
void m4_print(m4);
v4 m4v4mul(m4, v4);
m4 m4mul(m4, m4);
m4 m4_rotation_x(float theta);
m4 m4_rotation_y(float theta);
m4 m4_rotation_z(float theta);
m4 m4_translation(v3 t);

/* projection stuff */
m4 m4_perspective(float near, float far, float ratio, float fov);
v3 project(v4 v);
v3 clip_to_scr(v3 v, int win_w, int win_h);

/* shenanigans */
float rand_float(void);
