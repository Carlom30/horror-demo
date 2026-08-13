#include "../libs/SDL2/include/SDL2/SDL.h"
#include "../libs/SDL2/include/SDL2/SDL_timer.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "lmath.h"
#include "render.h"
#include "mesh.h"
#include "utils.h"
#include "obj.h"
#include "display.h"

/* TODO input system */
void camera_rotate(camera *cam, float speed)
{
	int dx, dy;
	SDL_GetRelativeMouseState(&dx, &dy);

	float xoff = -(float)dx * speed;
	float yoff = -(float)dy * speed;

	//printf("rels: [%f, %f]\n", xoff, yoff);
	cam->yaw += xoff;
	cam->pitch += yoff;

	float angle_max = 89.0f * M_PI / 180.0f;

	if (cam->pitch > angle_max ) cam->pitch = angle_max;
	if (cam->pitch < -angle_max ) cam->pitch = -angle_max;

	v3 dir = {0};
	dir.x = cos(cam->yaw) * cos(cam->pitch);
	dir.y = sin(cam->pitch);
	dir.z = sin(cam->yaw) * cos(cam->pitch);

	cam->dir = v3_norm(dir);
}

int main(void)
{
	int ww = 800;
	int wh = 600;
	display_init(ww, wh, "The Dolphin Hotel");
	render_init();
	int quit = 0;
	SDL_Event e;
	render_getwh(&ww, &wh);
	float dt = 0.0f;
	while (!quit) {
		/* TODO input system */
		delta_time_update();
		dt = delta_time();
		SDL_PumpEvents();
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = 1;
		}
		/* TODO: Input system */
		const uint8_t *state = SDL_GetKeyboardState(NULL);
		camera *cam = render_camera_ptr();
		if (state[SDL_SCANCODE_W]) {
			cam->pos = v3_sum(cam->pos, v3_mul_f(cam->dir, 10.0f * dt));
		}
		if (state[SDL_SCANCODE_S]) {
			cam->pos = v3_sum(cam->pos, v3_mul_f(cam->dir, -10.0f * dt));
		}
		if (state[SDL_SCANCODE_A]) {
			cam->pos = v3_sum(cam->pos, v3_mul_f(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam->dir)), -dt * 2.0f));
		}
		if (state[SDL_SCANCODE_D]) {
			cam->pos = v3_sum(cam->pos, v3_mul_f(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam->dir)), dt * 2.0f));
		}
		camera_rotate(cam, 0.005f);
		render_clear();
		triangle *trisproj = NULL;
		render_set_color(0, 200, 0);
		mesh *scene = render_scene();
		for (int c = 0; c < DA_COUNT(scene); c++) {
			DA_ALLOC(trisproj);
			mesh *m = &scene[c];
			vertex_shader((const mesh*)m, &trisproj);
			for (int i = 0; i < DA_COUNT(trisproj); i++)
				fragment_shader(trisproj[i], m->texture);
			DA_FREE(trisproj);
			m->theta += 0.3f * dt;
		}
		render_update();
	}
	return 0;
}
