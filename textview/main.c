/* standalone texture viewer
   build: cc -o texview main.c ../src/utils.c -I../src -I../libs/SDL2/include -L../libs/SDL2 -lSDL2 -lm
   run:   LD_LIBRARY_PATH=../libs/SDL2/ ./texview */

#include <SDL2/SDL.h>

#include "../src/utils.h"

#define TEXTURE_PATH "../assets/peng/PenguinText.png"

int main(void)
{
	png p = {0};
	if (png_load(TEXTURE_PATH, &p))
		ERROR("failed to load %s\n", TEXTURE_PATH);
	printf("loaded %s: %dx%d, %d channels\n", TEXTURE_PATH, p.w, p.h, p.n);

	if (SDL_Init(SDL_INIT_VIDEO))
		ERROR("SDL_Init: %s\n", SDL_GetError());

	SDL_Window *win = SDL_CreateWindow("peng texture",
					   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
					   p.w, p.h, 0);
	if (!win)
		ERROR("SDL_CreateWindow: %s\n", SDL_GetError());
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, 0);
	if (!ren)
		ERROR("SDL_CreateRenderer: %s\n", SDL_GetError());

	Uint32 fmt = p.n == 4 ? SDL_PIXELFORMAT_RGBA32 : SDL_PIXELFORMAT_RGB24;
	SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(p.data, p.w, p.h,
							       p.n * 8, p.w * p.n, fmt);
	if (!surf)
		ERROR("SDL_CreateRGBSurfaceWithFormatFrom: %s\n", SDL_GetError());
	SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surf);
	if (!tex)
		ERROR("SDL_CreateTextureFromSurface: %s\n", SDL_GetError());
	SDL_FreeSurface(surf);
	free(p.data);

	int running = 1;
	while (running) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				running = 0;
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
				running = 0;
		}
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, NULL, NULL);
		SDL_RenderPresent(ren);
	}

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
