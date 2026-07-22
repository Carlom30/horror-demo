/* examples: https://people.sc.fsu.edu/~jburkardt/data/obj/obj.html */

#include "obj.h"

#include <ctype.h>
#include <string.h>

#include "lmath.h"
#include "mesh.h"

typedef struct vert {
	v3 v;
	v3 uv;
} vert;

static v3 consume_vertex(const char *src, int *cursor)
{
	/* i assume the 'v' character was already found and skipped */
	int tmpc = *cursor;
	v3 v = {0};
	char c = src[tmpc];
	if (c != '-' && !isdigit(c)) {
		ERROR("obj_load_file file has broken vertex (char: %c)\n", c);
	}
	/* here we are sure the next number is a value */
	char *tmpsrc = (char *)src + tmpc;
	v.x = strtod(tmpsrc, &tmpsrc);
	tmpsrc++;
	v.y = strtod(tmpsrc, &tmpsrc);
	tmpsrc++;
	v.z = strtod(tmpsrc, &tmpsrc);
	tmpc = tmpsrc - src;
	*cursor = tmpc;
	return v;
}

static v3 consume_uv(const char *src, int *cursor)
{
	int tmpc = *cursor;
	v3 uv = {0};
	char c = src[tmpc];
	if (!isdigit(c)) {
		ERROR("obj_load_file file has broken uv (char: %c)\n", c);
	}
	char *tmpsrc = (char *)src + tmpc;
	uv.x = strtod(tmpsrc, &tmpsrc);
	tmpsrc++;
	uv.y = strtod(tmpsrc, &tmpsrc);
	tmpsrc++;
	tmpc = tmpsrc - src;
	*cursor = tmpc;
	return uv;
}

static int iii = 0;
static void consume_tri_idxs_and_uvs(const char *src, int *cursor, int **vidxs, int **uvidxs)
{
	int tmpc = *cursor;
	char c = src[tmpc];
	if (!isdigit(c)) {
		ERROR("obj_load_file file has broken indices (char: %c)\n", c);
	}
	int yes = 0;
	/* here we are sure the next number is a value */
	while (1) {
		if (src[tmpc] == '\n')
			break;
		char *tmpsrc = NULL;
		DA_APPEND(*vidxs, strtol(src + tmpc, &tmpsrc, 0));
		printf("last: %d\n", DA_LAST(*vidxs));
		tmpc = (tmpsrc - (src + tmpc)) + 1;
		/* printf("dif is %lu\n", tmpsrc - src); */
		/* printf("%d but %lu\n", tmpc, strlen(src)); */
		if (src[tmpc] == '/') {
			DA_APPEND(*uvidxs, strtol(src + tmpc, &tmpsrc, 0));
			tmpc = (tmpsrc - (src + tmpc)) + 1;
		}
		tmpc++;
	}
	if (yes) {
		for (int i = 0; i < DA_COUNT(*vidxs); i++)
			printf("%d ", (*vidxs)[i]);
		printf("\n");
		for (int i = 0; i < DA_COUNT(*uvidxs); i++)
			printf("%d ", (*uvidxs)[i]);
		printf("\n");
		iii++;
		if (iii == 3)
			assert(0);
	}
	*cursor = tmpc;
}

/* the obj format is probably not the way to go format, so im going to just assume that
   this parser will work, wont bother with error handling */
int obj_load_mesh(const char *path, mesh *dst)
{
	char *src = NULL;
	if (read_file_str(path, &src))
		ERROR("obj_load_file failed load file\n");
	int cursor = 0;
	v3 *verts = NULL;
	v3 *uvs = NULL;
	int *vidxs = NULL;
	int *uvidxs = NULL;
	DA_ALLOC(verts);
	DA_ALLOC(uvs);
	DA_ALLOC(vidxs);
	DA_ALLOC(uvidxs);
	while (1) {
		/* ok so first thing, vertices */
		char c = src[cursor];
		if (c == '\0')
			break;
		else if (c == '\n' || c == ' ' || c == '\t') {
			cursor++;
			continue;
		} else if (c == 'v' && src[cursor + 1] == ' ') {
			cursor += 2; /* skip v and space */
			v3 v = consume_vertex(src, &cursor);
			DA_APPEND(verts, v);
		} else if (c == 'v' && src[cursor + 1] == 't' && src[cursor + 2] == ' ') {
			cursor += 3;
			v3 uv = consume_uv(src, &cursor);
			DA_APPEND(uvs, uv);
		} else if (c == 'f' &&  src[cursor + 1] == ' ') {
			cursor += 2; /* skip v and space */
			consume_tri_idxs_and_uvs(src, &cursor, &vidxs, &uvidxs);
		}
		cursor++;
	}
	/* last, build the mesh */
	mesh m = mesh_alloc();
	for (int i = 0; i < DA_COUNT(vidxs); i += 3) {
		triangle t = {0};
		t.p0 = verts[vidxs[i + 0] - 1];
		t.p1 = verts[vidxs[i + 1] - 1];
		t.p2 = verts[vidxs[i + 2] - 1];
		DA_APPEND(m.tris, t);
	}
	printf("tris count: %d\n", DA_COUNT(vidxs) / 3);
	*dst = m;
	return NOERR;
}
