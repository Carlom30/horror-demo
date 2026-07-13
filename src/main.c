#include "lmath.h"
#include "render.h"
#include "loop.h"

int main(void)
{
	int ww = 800;
	int wh = 600;
	render_init(ww, wh, "game");
	loop_init();
	loop_main();
	return 0;
}
