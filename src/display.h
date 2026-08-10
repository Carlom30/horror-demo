#pragma once

#include <stdint.h>

int display_init(int win_w, int win_h, const char *name);
void display_update(uint32_t *buffer);
uint32_t display_map_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

