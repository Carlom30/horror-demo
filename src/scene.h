#pragma once

#include "lmath.h"
#include "mesh.h"

typedef struct component {
	void *args;
	void (*start)(void *args);
	void (*update)(void *args);
	void (*destroy)(void *args);
} component;

typedef struct game_object {
	enum mesh_name meshid;
	float theta; /* TODO implement axis rotation */
	v3 pos;
	v3 scale;
	component *components; /* dynamic array */
} game_object;

int scene_init();

game_object *scene_get();

/* return the id assigned to the new object on the scene */
uint32_t game_object_make(enum mesh_name meshid);
game_object *game_object_get_by_id(uint32_t id);
void game_object_append_component(uint32_t goid, component c);
m4 game_object_transform(game_object go);
