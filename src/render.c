#include "render.h"

#include <assert.h>
#include <string.h>

#include "lmath.h"
#include "utils.h"
#include "display.h"

#define SURFW 300
#define SURFH 225

static struct {
	int init;
	uint32_t *buffer;
	int fb_w;
	int fb_h;
	uint32_t color;
} render = {0};

#define COORD_OUT_BUF_BOUND(x, y) ((x) < 0 || (x) >= render.fb_w || (y) < 0 || (y) >= render.fb_h)

static inline int bufidx(int x, int y)
{
	return y * render.fb_w + x;
}

void rect_print(rect r)
{
	printf("x %d, y %d, w %d, h %d\n", r.x, r.y, r.w, r.h);
}

void render_getwh(int *w, int *h)
{
	if (!render.init) {
		printf("ERROR: render system was not initialized\n");
		assert(0);
	}
	*w = render.fb_w;
	*h = render.fb_h;
}

/* TODO: separate rendering surface from the window surface. This way, we can render less pixel and then
   blit the smaller surface scaled to the real window */
int render_init()
{
	/* now the real rendering surface, which will be half of the window size */
	render.fb_w = SURFW;
	render.fb_h = SURFH;
	render.buffer = malloc(sizeof(uint32_t) * (render.fb_w) * (render.fb_h));
	render.color = display_map_rgba(0, 0, 0, 255);
	render.init = 1;
	return 0;
}

void render_set_color(uint8_t r, uint8_t g, uint8_t b)
{
	render.color = display_map_rgba(r, g, b, 255);
}

void render_set_pixel(uint32_t p, int x, int y)
{
	/* note that this is not calling SDL_MapRGBA so make sure to pass a pixel
	   with the correct format */
	if (COORD_OUT_BUF_BOUND(x, y)) {
		ERROR("render_set_pixel coordinates out of bound\n");
	}
	render.buffer[bufidx(x, y)] = p;
}

void render_draw_rect(const rect *r)
{
	assert(r);
	for (int y = r->y; y < r->y + r->h; y++) {
		if (!COORD_OUT_BUF_BOUND(r->x, y))
			render.buffer[bufidx(r->x, y)] = render.color;
		if (!COORD_OUT_BUF_BOUND(r->x + r->w, y))
			render.buffer[bufidx(r->x + r->w, y)] = render.color;
	}
	for (int x = r->x; x < r->x + r->w; x++) {
		if (!COORD_OUT_BUF_BOUND(x, r->y))
			render.buffer[bufidx(x, r->y)] = render.color;
		if (!COORD_OUT_BUF_BOUND(x, r->y + r->h))
			render.buffer[bufidx(x, r->y + r->h)] = render.color;
	}
}

void render_fill_rect(const rect *r)
{
	assert(r);
	for (int y = r->y; y < r->y + r->h; y++)
		for (int x = r->x; x < r->x + r->w; x++) {
			if (COORD_OUT_BUF_BOUND(x, y))
				continue;
			render.buffer[bufidx(x, y)] = render.color;
		}
}

void render_draw_point(int x, int y)
{
	if (COORD_OUT_BUF_BOUND(x, y))
		return;
	render.buffer[bufidx(x, y)] = render.color;
}

/* Bresenham algorithm */
void render_plot_line(int x0, int y0, int x1, int y1)
{
	int dx = abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int error = dx + dy;
	while (1) {
		render_draw_point(x0, y0);
		int e2 = 2 * error;
		if (e2 >= dy) {
			if (x0 == x1) break;
			error += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			if (y0 == y1) break;
			error += dx;
			y0 += sy;
		}
	}
}

void render_clear()
{
	memset(render.buffer, 0x18, render.fb_w * render.fb_h * sizeof(uint32_t));
}

void render_update()
{
	display_update(render.buffer);
}
