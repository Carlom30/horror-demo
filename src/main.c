#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "../libs/SDL2/include/SDL2/SDL.h"
#include "../libs/SDL2/include/SDL2/SDL_timer.h"

#include "lmath.h"
#include "render.h"
#include "utils.h"
#include "mesh.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))


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
	camera cam = camera_init();
	v3 p0 = v3mk(0.0f, 0.0f, 4.0f);
	v3 p1 = v3mk(0.0f, 2.0f, 4.0f);
	v3 p2 = v3mk(2.0f, 0.0f, 4.0f);
	v3 p3 = v3mk(0.0f, 0.0f, 9.0f);
	v3 p4 = v3mk(0.0f, 7.0f, 1.0f);
	v3 p5 = v3mk(9.0f, 0.0f, 15.0f);

	mesh cubes[] = {
		mesh_cube(&p0),
		mesh_cube(&p1),
		mesh_cube(&p2),
		mesh_cube(&p3),
		mesh_cube(&p4),
		mesh_cube(&p5)
	};
	int ww = 1280;
	int wh = 720;
	render_init(ww, wh, "game");
	SDL_Event e;
	int quit = 0;
	int fps = 0;
	uint64_t now = SDL_GetTicks64();
	uint64_t old = 0;
	float one = 0;
	m4 persp = m4_perspective(0.1f, 100.0f, (float)wh / (float)ww, M_PI / 4.0f);
	float delta_time = 0.0f;
	while (!quit) {
		old = now;
		now = SDL_GetTicks64();
		one += ((float)now - (float)old) / 1000.0f;
		delta_time = ((float)now - (float)old) / 1000.0f;
		/* delta_time = 1.0f / 60.0f; */
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
		}
		const uint8_t *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_W]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(cam.dir, 2.0f * delta_time));
		}
		if (state[SDL_SCANCODE_S]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(cam.dir, -2.0f * delta_time));
		}
		if (state[SDL_SCANCODE_A]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam.dir)), -delta_time * 2.0f));
		}
		if (state[SDL_SCANCODE_D]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam.dir)), delta_time * 2.0f));
		}
		camera_rotate(&cam, 0.005f);
		render_clear();
		m4 view = m4_camera_view(cam);
		for (int c = 0; c < ARRAY_SIZE(cubes); c++) {
			mesh *cube = &cubes[c];
			mesh_render((const mesh*)cube, view, persp);
			cube->theta += 1.0f * delta_time;
		}
		render_update();
		/* SDL_Delay(delta_time * 1000.0f); */
	}
	return 0;
}
