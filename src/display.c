#include "display.h"

#include "../libs/SDL2/include/SDL2/SDL.h"
#include <stdlib.h>
#include <assert.h>

#define FRAME_BUFFER_W 300
#define FRAME_BUFFER_H 225

static struct {
	int init;
	int win_w;
	int win_h;
	int fb_w;
	int fb_h;
	SDL_Window *win;
	SDL_Surface *surface;
	SDL_Surface *tmpsurf;
} display;

int display_init(int win_w, int win_h, const char *name)
{
	if (((float)win_w / (float)win_h) != (4.0f / 3.0f)) {
		printf("ERROR: please initialize a 4/3 window\n");
		assert(0);
	}
	display.win_w = win_w;
	display.win_h = win_h;
	display.win = SDL_CreateWindow(
		name,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		win_w, win_h, 0);
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_WarpMouseInWindow(display.win, 0, 0);
	/* NOTE: for now these dimentions (fb) are scripted, but could become something else later*/
	display.fb_w = FRAME_BUFFER_W;
	display.fb_h = FRAME_BUFFER_H;
	display.surface = SDL_GetWindowSurface(display.win);
	display.tmpsurf = SDL_CreateRGBSurfaceWithFormat(0,
		display.fb_w,
		display.fb_h,
		sizeof(uint32_t),
		SDL_GetWindowSurface(display.win)->format->format);
	display.init = 1;
	return 0;
}

void display_update(uint32_t *frame)
{
	memcpy(display.tmpsurf->pixels, frame,
		sizeof(uint32_t) * display.fb_w * display.fb_h);
	SDL_BlitScaled(display.tmpsurf, NULL, display.surface, NULL);
	SDL_UpdateWindowSurface(display.win);
}

uint32_t display_map_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	return SDL_MapRGBA(display.surface->format, r, g, b, a);
}
