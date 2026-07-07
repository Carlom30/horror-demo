#include <stdio.h>
#include "../libs/SDL2/include/SDL2/SDL.h"
#include "../libs/SDL2/include/SDL2/SDL_timer.h"

#include "lmath.h"
#include "render.h"
#include "utils.h"
#include "mesh.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

int main(void)
{
	mesh cubes[] = {
		mesh_cube(),
		mesh_cube(),
		mesh_cube()
	};
	v3 poses[ARRAY_SIZE(cubes)] = {
		v3mk(0.0f, 0.0f, 5.0f),
		v3mk(2.0f, 0.0f, 5.0f),
		v3mk(0.0f, 2.0f, 5.0f)
	};
	int ww = 1280;
	int wh = 720;
	render_init(ww, wh, "game");
	SDL_Event e;
	int quit = 0;
	int x = 0;
	int y = 0;
	int fps = 0;
	uint64_t now = SDL_GetTicks64();
	uint64_t old = 0;
	float one = 0;
	m4 persp = m4_perspective(0.1f, 100.0f, (float)wh / (float)ww, M_PI / 4.0f);
	float dz = 2.0f;
	float theta = 0.0f;
	float delta_time = 0.0f;
	while (!quit) {
		old = now;
		now = SDL_GetTicks64();
		one += ((float)now - (float)old) / 1000.0f;
		delta_time = ((float)now - (float)old) / 1000.0f;
		if (one >= 1.0f) {
			printf("FPS: %d\n", fps);
			one = 0.0f;
			fps = 0;
		} else {
			fps++;
		}
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = 1;
			if (e.key.keysym.sym == SDLK_d)
				x++;
			else if (e.key.keysym.sym == SDLK_a)
				x--;
			else if (e.key.keysym.sym == SDLK_s)
				y++;
			else if (e.key.keysym.sym == SDLK_w)
				y--;
		}
		render_clear();
		render_set_color(0, 200, 0);
		for (int c = 0; c < ARRAY_SIZE(cubes); c++) {
			mesh cube = cubes[c];
			for (int i = 0; i < DA_COUNT(cube.tris); i++) {
				triangle tri = cube.tris[i];
				v4 p0 = v3v4(tri.p0);
				v4 p1 = v3v4(tri.p1);
				v4 p2 = v3v4(tri.p2);
				m4 tr = m4_rotation_y(theta);
				tr = m4mul(m4_rotation_x(theta), tr);
				tr = m4mul(m4_rotation_z(theta), tr);
				tr = m4mul(m4_translation(poses[c]), tr);
				tr = m4mul(persp, tr);
				p0 = m4v4mul(tr, p0);
				p1 = m4v4mul(tr, p1);
				p2 = m4v4mul(tr, p2);
				v3 pp0 = (clip_to_scr(project(p0), ww, wh));
				v3 pp1 = (clip_to_scr(project(p1), ww, wh));
				v3 pp2 = (clip_to_scr(project(p2), ww, wh));
				render_plot_line(pp0.x, pp0.y, pp1.x, pp1.y);
				render_plot_line(pp1.x, pp1.y, pp2.x, pp2.y);
				render_plot_line(pp2.x, pp2.y, pp0.x, pp0.y);
			}
		}
		theta += 1.0f * delta_time;
		render_update();
	}
	return 0;
}
