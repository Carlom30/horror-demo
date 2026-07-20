#include "loop.h"

#include <assert.h>
#include <limits.h>
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
static float *zbuffer = NULL;

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
	mesh_load_all();
	v3 p = v3mk(0.0f, 0.0f, 4.0f);
	mesh m = mesh_get_by_name(MN_COW);
	m.pos = p;
	DA_APPEND(scene, m);
	cam = camera_init();
	int w, h;
	render_getwh(&w, &h);
	zbuffer = calloc(w * h, sizeof(float));
}

camera loop_get_camera()
{
	return cam;
}

static float edge_function(v3 v0, v3 v1, v3 p)
{
	return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y ) * (v1.x - v0.x);
}

/* https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html */
static int w, h;
static int done = 0;
static void raster_triangle(triangle t)
{
	if (!done) {
		done = 1;
		render_getwh(&w, &h);
	}
	float tarea2 = 2.0f * triangle_area(t);
	if (tarea2 <= 0)
		return; /* this triangle is internal to the mesh, dont care */
	v3 ld = v3mk(0.0f, 0.0f, -1.0f);
	float dp = t.norm.x * ld.x + t.norm.y * ld.y + t.norm.z * ld.z;
	dp = CLAMP(dp, 0.0f, 1.0f);
	float grey = CLAMP(200.0f * dp, 10.0f, 200.0f);
	render_set_color(grey, grey, grey);
	rect r = find_triangle_box(t);
	int x0 = MAX(r.x, 0);
	int y0 = MAX(r.y, 0);
	int x1 = MIN(r.x + r.w, w);
	int y1 = MIN(r.y + r.h, h);
	for (int y = y0; y < y1; y++) {
		for (int x = x0; x < x1; x++) {
			v3 p = v3mk((float)x + 0.5f, (float)y + 0.5f, 0);
			float ep02 = edge_function(t.p0, t.p2, p);
			float ep21 = edge_function(t.p2, t.p1, p);
			float ep10 = edge_function(t.p1, t.p0, p);

			/* barycentric coordinates */
			float area2 = 2 * triangle_area(t);
			float w0 = ep21 / area2;
			float w1 = ep02 / area2;
			float w2 = ep10 / area2; /* 1.0f - (w0 + w1); */

			if (w0 < 0 || w1 < 0 || w2 < 0) continue;

			float z = w0 * t.p0.z + w1 * t.p1.z + w2 * t.p2.z;
			if (z >= zbuffer[y * w + x]) continue;

			zbuffer[y * w + x] = z;
			render_draw_point(x, y);
		}
	}
}

static void tris_raster(triangle *tris)
{
	assert(tris);
	for (int i = 0; i < DA_COUNT(tris); i++)
		raster_triangle(tris[i]);
}

static void mesh_project(const mesh *m, triangle **trisproj)
{
	int ww, wh;
	render_getwh(&ww, &wh);
	m4 mvp = mesh_transform(*m);
	/* m4 tr = mvp; */
	mvp = m4mul(view, mvp);
	mvp = m4mul(perspective, mvp);
	for (int i = 0; i < DA_COUNT(m->tris); i++) {
		triangle tri = m->tris[i];
		v4 p0 = v3v4(tri.p0);
		v4 p1 = v3v4(tri.p1);
		v4 p2 = v3v4(tri.p2);
		p0 = m4v4mul(mvp, p0);
		p1 = m4v4mul(mvp, p1);
		p2 = m4v4mul(mvp, p2);
		if (p0.w <= 0.1f || p1.w <= 0.1f || p2.w <= 0.1f)
			continue;
		v3 norm = triangle_normal(trimk(
			v4v3(p0),
			v4v3(p1),
			v4v3(p2))
		);
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
		for (int i = 0; i < wh; i++)
			for (int j = 0; j < ww; j++)
				zbuffer[i * ww + j] = FLT_MAX;
		view = m4_camera_view(cam);
		triangle *trisproj = NULL;
		render_set_color(0, 200, 0);
		for (int c = 0; c < DA_COUNT(scene); c++) {
			mesh *m = &scene[c];
			DA_ALLOC(trisproj);
			mesh_project((const mesh*)m, &trisproj);
			tris_raster(trisproj);
			m->theta += 1.0f * delta_time;
			DA_FREE(trisproj);
		}
		render_update();
	}
	return;
}

