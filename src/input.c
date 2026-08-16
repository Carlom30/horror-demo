#include "input.h"

#include "../libs/SDL2/include/SDL2/SDL.h"

#include "utils.h"

static input_state is;
static uint8_t ks_last[NUM_SCANCODES];

int input_init()
{
	memset(is.ks, KS_NOTHING, NUM_SCANCODES);
	memset(ks_last, KS_NOTHING, NUM_SCANCODES);
	return NOERR;
}

static char *ks_str[] = {
	"KS_NOTHING",
	"KS_PRESS",
	"KS_RELEASE",
	"KS_KEEP",
};

void key_state_print(enum key_state ks)
{
	printf("%s\n", ks_str[ks]);
}

void input_update()
{
	const uint8_t *sdl_state = SDL_GetKeyboardState(NULL);
	memcpy(ks_last, is.ks, NUM_SCANCODES);
	for (scancode sc = 0; sc < NUM_SCANCODES; sc++) {
		if (ks_last[sc] == KS_NOTHING && sdl_state[sc])
			is.ks[sc] = KS_PRESS;
		else if (ks_last[sc] == KS_PRESS && sdl_state[sc])
			is.ks[sc] = KS_KEEP;
		else if ((ks_last[sc] == KS_KEEP || ks_last[sc] == KS_PRESS)
			&& !sdl_state[sc])
			is.ks[sc] = KS_RELEASE;
		else if (ks_last[sc] == KS_RELEASE && !sdl_state[sc])
			is.ks[sc] = KS_NOTHING;
	}
}

const input_state input_state_get()
{
	return is;
}
