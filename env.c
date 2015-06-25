#include "env.h"
#include <stdlib.h>

struct Env make_env(void)
{
	struct Env e = {.map = 0, .count = 0, .size = 0, .parent = 0};
	return e;
}

struct Object *env_get(struct Env *env, ptrdiff_t sym)
{
	/* NOTE: this one is recursive */
	while (env) {
		for (ptrdiff_t i = 0; i != env->count; ++i)
			if (sym == env->map[i].key)
				return env->map[i].value;
		env = env->parent;
	}
	return 0;
}

ptrdiff_t env_search(struct Env *env, ptrdiff_t sym)
{
	/* NOTE: this one is not recursive. */
	for (ptrdiff_t i = 0; i != env->count; ++i)
		if (sym == env->map[i].key)
			return i;
	return -1;
}

bool env_map_append(struct Env *env, struct EnvEntry ent)
{
	if (env->count >= env->size) {
		size_t nsize = (env->size + 8) * sizeof(struct EnvEntry);
		struct EnvEntry *nmap = realloc(env->map, nsize);
		if (!nmap)
			return false;
		env->map = nmap;
		env->size = nsize;
	}
	(env->map)[(env->count)++] = ent;
	return true;
}

bool env_update(struct Env *env, ptrdiff_t sym, struct Object *obj)
{
	/* NOTE: this one is not recursive.  This will overwrite an
	 * entry in the Env passed in, but will shadow an entry in a
	 * parent Env.
	 */
	ptrdiff_t i = env_search(env, sym);
	if (i == -1) {
		struct EnvEntry ent = {.key = sym, .value = obj};
		return env_map_append(env, ent);
	}
	env->map[i].value = obj;
	return true;
}
