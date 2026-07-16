#include <time.h>
#include <stdlib.h>

#include "lmath.h"
#include "render.h"
#include "loop.h"
#include "mesh.h"

int main(void)
{
	srand(time(NULL));
	int ww = 800;
	int wh = 600;
	render_init(ww, wh, "game");
	loop_init();
	for (int i = 0; i < 10; i++) {
		v3 rp = v3mk(rand() % 20, rand() % 20, rand() % 20);
		scene_append_object(mesh_cube(&rp));
	}
	loop_main();
	return 0;
}
