#include "loop.h"

#include "../libs/SDL2/include/SDL2/SDL.h"
#include "../libs/SDL2/include/SDL2/SDL_timer.h"

#include "utils.h"
#include "mesh.h"
#include "lmath.h"
#include "render.h"

static mesh *scene = NULL;
static float delta_time = 0.0f;
static camera cam;
static m4 perspective;
static m4 view;

/* TODO: move this in a input manager system */
static SDL_Event e;

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

void loop_init()
{
	DA_ALLOC(scene);
	v3 p = v3mk(0.0f, 0.0f, 2.0f);
	DA_APPEND(scene, mesh_cube(&p));
	cam = camera_init();
}

camera loop_get_camera()
{
	return cam;
}

void mesh_project(const mesh *m, triangle **trisproj)
{
	DA_ALLOC(*trisproj);
	int ww, wh;
	render_getwh(&ww, &wh);
	m4 tr = mesh_transform(*m);
	tr = m4mul(view, tr);
	tr = m4mul(perspective, tr);
	for (int i = 0; i < DA_COUNT(m->tris); i++) {
		triangle tri = m->tris[i];
		v4 p0 = v3v4(tri.p0);
		v4 p1 = v3v4(tri.p1);
		v4 p2 = v3v4(tri.p2);
		p0 = m4v4mul(tr, p0);
		p1 = m4v4mul(tr, p1);
		p2 = m4v4mul(tr, p2);
		v3 norm = triangle_normal(trimk(
			v4v3(p0),
			v4v3(p1),
			v4v3(p2))
		);
		if (p0.w <= 0.1f || p1.w <= 0.1f || p2.w <= 0.1f) continue;
		v3 pp0 = (clip_to_scr(project(p0), ww, wh));
		v3 pp1 = (clip_to_scr(project(p1), ww, wh));
		v3 pp2 = (clip_to_scr(project(p2), ww, wh));
		triangle pjt = trimk(pp0, pp1, pp2);
		pjt.norm = norm;
		DA_APPEND(*trisproj, pjt);
		/* render_draw_rect(&pr); */
	}
}

void scene_append_object(mesh obj)
{
	DA_APPEND(scene, obj);
}

void loop_main()
{
	int quit = 0;
	int fps = 0;
	uint64_t now = SDL_GetTicks64();
	uint64_t old = 0;
	float one = 0;
	int ww, wh;
	render_getwh(&ww, &wh);
	perspective = m4_perspective(0.1f, 100.0f, (float)wh / (float)ww, M_PI / 4.0f);
	while (!quit) {
		/* TODO input system */
		SDL_PumpEvents();
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
		}
		/* TODO: Input system */
		const uint8_t *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_W]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(cam.dir, 10.0f * delta_time));
		}
		if (state[SDL_SCANCODE_S]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(cam.dir, -10.0f * delta_time));
		}
		if (state[SDL_SCANCODE_A]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam.dir)), -delta_time * 2.0f));
		}
		if (state[SDL_SCANCODE_D]) {
			cam.pos = v3_sum(cam.pos, v3_times_scalar(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam.dir)), delta_time * 2.0f));
		}
		camera_rotate(&cam, 0.005f);
		render_clear();
		view = m4_camera_view(cam);
		triangle *trisproj = NULL;
		render_set_color(0, 200, 0);
		for (int c = 0; c < DA_COUNT(scene); c++) {
			mesh *m = &scene[c];
			mesh_project((const mesh*)m, &trisproj);
			for (int i = 0; i < DA_COUNT(trisproj); i++) {
				raster_triangle(trisproj[i]);
			}
			m->theta += 1.0f * delta_time;
		}
		render_update();
	}
	return;
}
