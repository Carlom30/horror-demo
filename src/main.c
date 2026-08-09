#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "lmath.h"
#include "render.h"
#include "loop.h"
#include "mesh.h"
#include "utils.h"
#include "obj.h"
#include "display.h"

int main(void)
{
	int ww = 800;
	int wh = 600;
	display_init(ww, wh, "The Dolphin Hotel");
	render_init();
	loop_init();
	loop_main();
	return 0;
}
