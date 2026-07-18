/* examples: https://people.sc.fsu.edu/~jburkardt/data/obj/obj.html */

#include "obj.h"

#include <ctype.h>

#include "lmath.h"
#include "mesh.h"

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
	char *tmpsrc = src + tmpc;
	v.x = strtod(tmpsrc, &tmpsrc);
	tmpsrc++;
	v.y = strtod(tmpsrc, &tmpsrc);
	tmpsrc++;
	v.z = strtod(tmpsrc, &tmpsrc);
	tmpc = tmpsrc - src;
	*cursor = tmpc;
	return v;
}

static void consume_tri_idxs(const char *src, int *cursor, int **dst)
{
	/* i assume the 'v' character was already found and skipped */
	int tmpc = *cursor;
	char c = src[tmpc];
	if (!isdigit(c)) {
		ERROR("obj_load_file file has broken indices (char: %c)\n", c);
	}
	/* here we are sure the next number is a value */
	char *tmpsrc = src + tmpc;
	DA_APPEND(*dst, strtol(tmpsrc, &tmpsrc, 0));
	tmpsrc++;
	DA_APPEND(*dst, strtol(tmpsrc, &tmpsrc, 0));
	tmpsrc++;
	DA_APPEND(*dst, strtol(tmpsrc, &tmpsrc, 0));
	tmpc = tmpsrc - src;
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
	int *idxs = NULL;
	DA_ALLOC(verts);
	DA_ALLOC(idxs);
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
		} else if (c == 'f' &&  src[cursor + 1] == ' ') {
			cursor += 2; /* skip v and space */
			consume_tri_idxs(src, &cursor, &idxs);
		}
		cursor++;
	}
	/* last, build the mesh */
	mesh m = mesh_init();
	for (int i = 0; i < DA_COUNT(idxs); i += 3) {
		triangle t = {0};
		t.p0 = verts[idxs[i + 0] - 1];
		t.p1 = verts[idxs[i + 1] - 1];
		t.p2 = verts[idxs[i + 2] - 1];
		DA_APPEND(m.tris, t);
	}
	printf("tris count: %d\n", DA_COUNT(idxs) / 3);
	*dst = m;
	return NOERR;
}
