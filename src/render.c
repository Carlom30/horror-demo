#include "render.h"

#include "../libs/SDL2/include/SDL2/SDL.h"
#include <assert.h>

#include "lmath.h"

static struct {
	SDL_Window *win;
	uint32_t *buffer;
	SDL_Surface *tmpsurf;
	int surf_w;
	int surf_h;
	int win_w;
	int win_h;
	uint32_t color;
} render;

#define COORD_OUT_BUF_BOUND(x, y) ((x) < 0 || (x) >= render.surf_w || (y) < 0 || (y) >= render.surf_h)

static inline int bufidx(int x, int y)
{
	return y * render.surf_w + x;
}

void rect_print(rect r)
{
	printf("x %d, y %d, w %d, h %d\n", r.x, r.y, r.w, r.h);
}

void render_getwh(int *w, int *h)
{
	*w = render.surf_w;
	*h = render.surf_h;
}

/* TODO: separate rendering surface from the window surface. This way, we can render less pixel and then
   blit the smaller surface scaled to the real window */
int render_init(int win_w, int win_h, const char *name)
{
	SDL_Init(SDL_INIT_VIDEO);
	render.win = SDL_CreateWindow(
		name,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		win_w, win_h, 0);
	render.win_w = win_w;
	render.win_h = win_h;
	render.color = SDL_MapRGBA(SDL_GetWindowSurface(render.win)->format,
				0, 0, 0, 255);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_WarpMouseInWindow(render.win, 0, 0);

	/* now the real rendering surface, which will be half of the window size */
	render.surf_w = win_w / 2;
	render.surf_h = win_h / 2;
	render.buffer = malloc(sizeof(uint32_t) * (render.surf_w) * (render.surf_h));
	render.tmpsurf = SDL_CreateRGBSurfaceWithFormat(0,
		render.surf_w,
		render.surf_h,
		sizeof(uint32_t),
		SDL_GetWindowSurface(render.win)->format->format);
	return 0;
}

void render_set_color(uint8_t r, uint8_t g, uint8_t b)
{
	render.color = SDL_MapRGBA(SDL_GetWindowSurface(render.win)->format,
				r, g, b, 255);
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
	memset(render.buffer, 0, render.surf_w * render.surf_h * sizeof(uint32_t));
}

void render_update()
{
	SDL_Surface *winsur = SDL_GetWindowSurface(render.win);
	memcpy(render.tmpsurf->pixels, render.buffer,
		sizeof(uint32_t) * render.surf_w * render.surf_h);
	SDL_BlitScaled(render.tmpsurf, NULL, winsur, NULL);
	SDL_UpdateWindowSurface(render.win);
}
