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
	v3 p = v3mk(0.0f, -13.0f, 50.0f);
	mesh m = mesh_get_by_name(MN_SKULL);
	for (int i = 0; i < DA_COUNT(m.tris); i++) {
		m.tris[i].p0col = v3mk(255, 255, 255);
		m.tris[i].p1col = v3mk(255, 255, 255);
		m.tris[i].p2col = v3mk(255, 255, 255);
	}
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

static inline v3 texture_affine_map(triangle t, float w0, float w1, float w2)
{
	v3 m = {0};
	m.x = t.uv0.x * w0 + t.uv1.x * w1 + t.uv2.x * w2;
	m.y = t.uv0.y * w0 + t.uv1.y * w1 + t.uv2.y * w2;
	return m;
}

/* https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html */
static int w, h;
static int done = 0;
static void raster_triangle(triangle t, png texture)
{
	if (!done) {
		done = 1;
		render_getwh(&w, &h);
	}
	float tarea2 = 2.0f * triangle_area(t);
	if (tarea2 <= 0)
		return; /* this triangle is internal to the mesh, dont care */
	/* if (t.p0.z >= 1.0f || t.p1.z >= 1.0f || t.p2.z >= 1.0f) */
	/* 	return; */
	v3 ld = v3mk(0.0f, 0.0f, -1.0f);
	float dp = t.norm.x * ld.x + t.norm.y * ld.y + t.norm.z * ld.z;
	dp = CLAMP(dp, 0.0f, 1.0f);
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
			if (z > 1.0f) continue;
			zbuffer[y * w + x] = z;

			v3 textcoord = texture_affine_map(t, w0, w1, w2);
			textcoord.x = textcoord.x * texture.w;
			textcoord.y = textcoord.y * texture.h;
			int tx = CLAMP(textcoord.x, 0, (texture.w - 1));
			int ty = CLAMP((texture.w - textcoord.y), 0, (texture.h - 1));
			color_t texel = ((color_t *)texture.data)[ty * texture.w + tx];
			render_set_color(texel.r * dp, texel.g * dp, texel.b * dp);
			render_draw_point(x, y);
		}
	}
}

static void tris_raster(triangle *tris)
{
	assert(tris);
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
		triangle pjt = tri;
		pjt.p0 = pp0;
		pjt.p1 = pp1;
		pjt.p2 = pp2;
		pjt.norm = norm;
		DA_APPEND(*trisproj, pjt);
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
			cam.pos = v3_sum(cam.pos, v3_mul_f(cam.dir, 10.0f * delta_time));
		}
		if (state[SDL_SCANCODE_S]) {
			cam.pos = v3_sum(cam.pos, v3_mul_f(cam.dir, -10.0f * delta_time));
		}
		if (state[SDL_SCANCODE_A]) {
			cam.pos = v3_sum(cam.pos, v3_mul_f(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam.dir)), -delta_time * 2.0f));
		}
		if (state[SDL_SCANCODE_D]) {
			cam.pos = v3_sum(cam.pos, v3_mul_f(v3_norm(cross_product(v3mk(0.0f, 1.0f, 0.0f), cam.dir)), delta_time * 2.0f));
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
			DA_ALLOC(trisproj);
			mesh *m = &scene[c];
			mesh_project((const mesh*)m, &trisproj);
			for (int i = 0; i < DA_COUNT(trisproj); i++)
				raster_triangle(trisproj[i], m->texture);
			DA_FREE(trisproj);
			m->theta += 0.3f * delta_time;
		}
		render_update();
	}
	return;
}

