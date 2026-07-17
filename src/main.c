#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "lmath.h"
#include "render.h"
#include "loop.h"
#include "mesh.h"

int main(void)
{
	int ww = 800;
	int wh = 600;
	render_init(ww, wh, "The Dolphin Hotel");
	loop_init();
	for (int i = 0; i < 0; i++) {
		v3 rp = v3mk(rand() % 20, rand() % 20, rand() % 20);
		scene_append_object(mesh_cube(&rp));
	}
	loop_main();
	return 0;
}
