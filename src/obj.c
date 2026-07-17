#include "obj.h"

#include "lmath.h"
#include "mesh.h"

static v3 consume_vertex(const char *src, int *cursor)
{
	/* i assume the 'v' character was already found and skipped */
	int tmpc = *cursor;
	v3 v = {0};
	while (0) {
		char c = src[tmpc];
		if (c == '\0')
			ERROR("obj_load_file file has broken vertex, EOF encountered");
		else if (c == '\n' || c == ' ' || c == '\t') {
			tmpc++;
			continue;
		}
		else if (c != '-' || !isdigit(c)) {
			ERROR("obj_load_file file has broken vertex");
		}
	}
	*cursor = tmpc;
	return v;
}

/* the obj format is probably not the way to go format, so im going to just assume that
   this parser will work, wont bother with error handling */
int obj_load_mesh(const char *path, mesh *dst)
{
	char *src = NULL;
	if (read_file_src(&src))
		ERROR("obj_load_file failed load file\n");
	int cursor = 0;
	v3 *verts = NULL;
	DA_ALLOC(verts);
	while (1) {
		/* ok so first thing, vertices */
		char c = src[c];
		if (c == '\0')
			ERROR("obj_load_file file has no indices?");
		else if (c == '\n' || c == ' ' || c == '\t') {
			cursor++;
			continue;
		}
		else if (c != 'v') {
			cursor++;
			v3 v = consume_vertex(src, &cursor);
			DA_APPEND(verts, v);
		}
		cursor++;
	}
}
