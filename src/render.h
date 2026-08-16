#pragma once

#include <stdint.h>

#include "utils.h"
#include "lmath.h"

typedef struct mesh mesh;
typedef struct triangle triangle;
typedef struct game_object game_object;

typedef struct rect {
	int x, y;
	int w, h;
} rect;

typedef union {
	uint32_t rgba;
	struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};
} color_t;

int render_init();
void render_getwh(int *w, int *h);
void render_set_color(uint8_t r, uint8_t g, uint8_t b);
void render_set_pixel(uint32_t p, int x, int y);
void render_draw_point(int x, int y);
void render_plot_line(int x0, int y0, int x1, int y1);
void render_draw_rect(const rect *r);
void render_fill_rect(const rect *r);
void render_clear();
void render_update();

/* 3D stuff */
void scene_append_object(mesh obj);

/* move this on a dedicated module */
struct camera camera_init();
void delta_time_update();
float delta_time();
struct camera *render_camera_ptr();
void render_scene();
void vertex_shader(const game_object *go, triangle **trisproj);
void fragment_shader(triangle t, image texture);
