#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "lmath.h"
#include "render.h"
#include "loop.h"
#include "mesh.h"
#include "utils.h"
#include "obj.h"

int main(void)
{
	char *cube = NULL;
	mesh m = {0};
	if (obj_load_mesh("assets/airboat.obj", &m) == ERR)
		ERROR("failed to load mesh\n");
	int ww = 800;
	int wh = 600;
	render_init(ww, wh, "The Dolphin Hotel");
	loop_init();
	for (int i = 0; i < 1; i++) {
		v3 rp = v3mk(rand() % 20, rand() % 20, rand() % 20);
		m.pos = rp;
		scene_append_object(m);
	}
	loop_main();
	return 0;
}
