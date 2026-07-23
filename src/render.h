#pragma once

#include <stdint.h>

typedef struct {
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

int render_init(int win_w, int win_h, const char *name);
void render_getwh(int *w, int *h);
void render_set_color(uint8_t r, uint8_t g, uint8_t b);
void render_set_pixel(uint32_t p, int x, int y);
void render_draw_point(int x, int y);
void render_plot_line(int x0, int y0, int x1, int y1);
void render_draw_rect(const rect *r);
void render_fill_rect(const rect *r);
void render_clear();
void render_update();
