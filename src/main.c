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
	int ww = 800;
	int wh = 600;
	render_init(ww, wh, "The Dolphin Hotel");
	loop_init();
	loop_main();
	return 0;
}
