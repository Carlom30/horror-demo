#include "scene.h"

#include "utils.h"
#include "lmath.h"
#include "input.h"

static game_object *scene;

void test_update(void *args)
{
	input_state is = input_state_get();
	if (is.ks[SCANCODE_H] & (KS_PRESS | KS_RELEASE))
		printf("h pressed on icosphere!\n");
}

int scene_init()
{
	DA_ALLOC(scene);
	uint32_t goid = game_object_make(MN_ICOSPHERE);
	game_object *go = game_object_get_by_id(goid);
	go->update = test_update;
	go->pos = v3mk(0.0f, 0.0f, 10.0f);
	return 0;
}

game_object *scene_get()
{
	return scene;
}

uint32_t game_object_make(enum mesh_name meshid)
{
	game_object go = (game_object){
		.meshid = meshid,
		.theta = 0.0f,
		.pos = v3mk(0.0f, 0.0f, 0.0f),
		.scale = v3mk(1.0f, 1.0f, 1.0f),
	};
	DA_APPEND(scene, go);
	return (DA_COUNT(scene) - 1);
}

game_object *game_object_get_by_id(uint32_t id)
{
	assert(id < DA_COUNT(scene));
	return &scene[id];
}

m4 game_object_transform(game_object go)
{
	m4 tr = m4_rotation_y(go.theta);
	tr = m4mul(m4_rotation_y(go.theta), tr);
	tr = m4mul(m4_translation(go.pos), tr);
	tr = m4mul(m4_scale(go.scale), tr);
	return tr;
}

