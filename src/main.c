#include "lmath.h"
#include "render.h"
#include "loop.h"

int main(void)
{
	int ww = 1280;
	int wh = 720;
	render_init(ww, wh, "game");
	loop_init();
	loop_main();
	return 0;
}
