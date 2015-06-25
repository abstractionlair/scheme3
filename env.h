#ifndef ENV_H
#define ENV_H

#include "scheme_forward.h"
#include <stdbool.h>
#include <stddef.h>

struct EnvEntry {
	ptrdiff_t key;
	struct Object *value;
};

struct Env {
	struct EnvEntry *map;
	size_t count;
	size_t size;
	struct Env *parent;
};

struct Env make_env(void);
struct Object *env_get(struct Env *env, ptrdiff_t sym);
ptrdiff_t env_search(struct Env *env, ptrdiff_t sym);
bool env_map_append(struct Env *env, struct EnvEntry ent);
bool env_update(struct Env *env, ptrdiff_t sym, struct Object *obj);


#endif
