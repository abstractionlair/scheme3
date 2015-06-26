#include "eval.h"
#include "scheme.h"
#include <assert.h>
#include <stdio.h>

struct Object *eval_pair(struct Machine *machine, struct Object *obj);

struct Object *eval(struct Machine *machine, struct Object *obj)
{
	struct Object *nobj;
	switch (obj->type) {
	case TypeString:
	case TypeInteger:
	case TypeDouble:
	case TypeError:
	case TypeEnv:
	case TypeBuiltinForm:
		return obj;
	case TypeSymbol:
		nobj = env_get(&machine->rootEnv->env, obj->symbol);
		if (nobj) {
			return nobj;
		} else {
			char *symStr = machine->symbols.strs[obj->symbol].cstr;
			fprintf(stderr,
				"Failed to find %s in environment.\n",
				symStr);
			return create_error_object(machine);
		}
	case TypePair:
		return eval_pair(machine, obj);
	}
	assert(0);
	return 0;
}

struct Object *eval_pair(struct Machine *machine, struct Object *obj)
{
	if (obj->pair.car) {
		struct Object *ecar = eval(machine, obj->pair.car);
		if (ecar->type == TypeBuiltinForm)
			return ecar->builtinForm.f(machine, obj->pair.cdr);
	}
	fprintf(stderr, "Function calls not implemented yet\n");
	return create_error_object(machine);
}

