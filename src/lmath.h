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

/* is this a pure mathematical object? */
typedef struct {
	v3 pos;
	v3 dir;
	float yaw;
	float pitch;
} camera;

void v3_print(v3);
void v4_print(v4);
v4 v4mk(float, float, float, float);
v3 v3mk(float, float, float);
v4 v3v4(v3);
v3 v4v3(v4);
v3 cross_product(v3, v3);
float v3_magn(v3);
v3 v3_norm(v3);
v3 v3_sum(v3, v3);
v3 v3_sub(v3, v3);
v3 v3_times_scalar(v3, float);

/* matrix */
m4 m4_identity();
void m4_print(m4);
v4 m4v4mul(m4, v4);
m4 m4mul(m4, m4);
m4 m4_rotation_x(float theta);
m4 m4_rotation_y(float theta);
m4 m4_rotation_z(float theta);
m4 m4_translation(v3);

/* projection stuff */
m4 m4_perspective(float near, float far, float ratio, float fov);
v3 project(v4 v);
v3 clip_to_scr(v3 v, int win_w, int win_h);

/* camera */
camera camera_mk(v3 pos, v3 dir);

/* return a 0 pos z directioned camera */
camera camera_init();
m4 m4_camera_view(camera c);

/* shenanigans */
float rand_float(void);
