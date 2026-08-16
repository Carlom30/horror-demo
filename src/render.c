#include "render.h"
#include "../libs/SDL2/include/SDL2/SDL.h"
#include "../libs/SDL2/include/SDL2/SDL_timer.h"
#include <assert.h>
#include <string.h>

#include "lmath.h"
#include "utils.h"
#include "display.h"
#include "mesh.h"
#include "scene.h"

#define SURFW 300
#define SURFH 225

static struct {
	int init;
	uint32_t *buffer;
	int fb_w;
	int fb_h;
	uint32_t color;
} render = {0};

/* fps stuff
   maybe make a module for this */
static int fps = 0;
static uint64_t now = 0;
static uint64_t old = 0;
static float one = 0;
static float dt = 0.0f;

static struct camera cam = {0};
static m4 perspective;
static m4 view;
static float *zbuffer = NULL;

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

struct camera camera_init()
{
	return (struct camera){
		(v3){0}, (v3){ 0.0f, 0.0f, 1.0f },
		.yaw = M_PI / 2.0f,
	};
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
	mesh_load_all();
	perspective = m4_perspective(0.1f, 100.0f, (float)render.fb_h / (float)render.fb_w, M_PI / 4.0f);
	cam = camera_init();
	/* dead code below? maybe remove render_getwh... */
	int w, h;
	render_getwh(&w, &h);
	zbuffer = calloc(w * h, sizeof(float));
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
	for (int i = 0; i < render.fb_h; i++)
		for (int j = 0; j < render.fb_w; j++)
			zbuffer[i * render.fb_w + j] = FLT_MAX;
	/* (TODO) i think this does not belongs here, move it away */
	view = m4_camera_view(cam);
}

void render_update()
{
	display_update(render.buffer);
}

/* 3D stuff */
static inline float edge_function(v3 v0, v3 v1, v3 p)
{
	return (p.x - v0.x) * (v1.y - v0.y) - (p.y - v0.y ) * (v1.x - v0.x);
}

static inline color_t color_barycentric(triangle t, float w0, float w1, float w2)
{
	color_t c = {0};
	c.r = t.p0col.x * w0 + t.p1col.x * w1 + t.p2col.x * w2;
	c.g = t.p0col.y * w0 + t.p1col.y * w1 + t.p2col.y * w2;
	c.b = t.p0col.z * w0 + t.p1col.z * w1 + t.p2col.z * w2;
	c.a = 255;
	return c;
}

static inline v3 texture_affine_map(triangle t, float w0, float w1, float w2)
{
	v3 m = {0};
	m.x = t.uv0.x * w0 + t.uv1.x * w1 + t.uv2.x * w2;
	m.y = t.uv0.y * w0 + t.uv1.y * w1 + t.uv2.y * w2;
	return m;
}

static int w, h;
static int done = 0;
void fragment_shader(triangle t, image texture)
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

			if (texture.data) {
				v3 textcoord = texture_affine_map(t, w0, w1, w2);
				textcoord.x = textcoord.x * texture.w;
				textcoord.y = textcoord.y * texture.h;
				int tx = CLAMP(textcoord.x, 0, (texture.w - 1));
				int ty = CLAMP((texture.h - textcoord.y), 0, (texture.h - 1));
				color_t texel = ((color_t *)texture.data)[ty * texture.w + tx];
				render_set_color(texel.r * dp, texel.g * dp, texel.b * dp);
				render_draw_point(x, y);
			} else {
				color_t c = color_barycentric(t, w0, w1, w2);
				render_set_color(c.r * dp, c.g * dp, c.b * dp);
				render_draw_point(x, y);
			}
		}
	}
}

void vertex_shader(const game_object *go, triangle **trisproj)
{
	int ww, wh;
	render_getwh(&ww, &wh);
	m4 mvp = game_object_transform(*go);
	mesh m = mesh_get_by_name(go->meshid);
	/* m4 tr = mvp; */
	mvp = m4mul(view, mvp);
	mvp = m4mul(perspective, mvp);
	for (int i = 0; i < DA_COUNT(m.tris); i++) {
		triangle tri = m.tris[i];
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

float delta_time()
{
	return dt;
}

void delta_time_update()
{
	old = now;
	now = SDL_GetTicks64();
	one += ((float)now - (float)old) / 1000.0f;
	dt = ((float)now - (float)old) / 1000.0f;
	if (one >= 1.0f) {
		printf("FPS: %d\n", fps);
		one = 0.0f;
		fps = 0;
	} else {
		fps++;
	}
}

void render_scene()
{
	game_object *scene = scene_get();
	triangle *trisproj = NULL;
	for (int c = 0; c < DA_COUNT(scene); c++) {
		game_object *go = &scene[c];
		mesh m = mesh_get_by_name(go->meshid);
		DA_ALLOC(trisproj);
		vertex_shader((const game_object *)go, &trisproj);
		for (int i = 0; i < DA_COUNT(trisproj); i++)
			fragment_shader(trisproj[i], m.texture);
		DA_FREE(trisproj);
		go->theta += 0.3f * dt;
	}
}

struct camera *render_camera_ptr()
{
	return &cam;
}
