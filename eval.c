#include "eval.h"
#include "scheme.h"
#include "print.h"
#include <assert.h>
#include <stdio.h>

struct Object *eval_pair(struct Machine *machine, struct Object *obj);

struct Object *eval(struct Machine *machine, struct Object *obj)
{
	struct Object *nobj = 0;
	switch (obj->type) {
	case TypeString:
	case TypeInteger:
	case TypeDouble:
	case TypeError:
	case TypeEnv:
	case TypeBuiltinForm:
	case TypeBuiltinFunc:
		nobj = obj;
		break;
	case TypeSymbol:
		nobj = env_get(&machine->env->env, obj->symbol);
		if (!nobj) {
			char *symStr = machine->symbols.strs[obj->symbol].cstr;
			fprintf(stderr,
				"Failed to find %s in environment.\n",
				symStr);
			nobj = create_error_object(machine);
		}
		break;
	case TypePair:
		nobj = eval_pair(machine, obj);
		break;
	default:
		assert(0);
	}
	return nobj;
}

struct Object *eval_list_items(struct Machine *machine, struct Object *inList)
{
	struct Object *first = create_pair_object(machine, 0, 0);
	if (!first)
		return 0;
	if (obj_is_nil(inList))
		return first;
	struct Object *into = first;
	do {
		into->pair.car = eval(machine, car(inList));
		into->pair.cdr = create_pair_object(machine, 0, 0);
		if (!into->pair.cdr)
			return 0;
		into = cdr(into);
		inList = cdr(inList);
	} while (!obj_is_nil(inList));
	return first;
}

struct Object *eval_pair(struct Machine *machine, struct Object *obj)
{
	struct Object *nobj = 0;
	if (obj->pair.car) {
		struct Object *ecar = eval(machine, obj->pair.car);
		if (ecar->type == TypeBuiltinForm) {
			nobj = ecar->builtinForm.f(machine, obj->pair.cdr);
		} else if (ecar->type == TypeBuiltinFunc) {
			struct Object *eargs = eval_list_items(machine,
							obj->pair.cdr);
			nobj = ecar->builtinFunc.f(machine, eargs);
		} else {
			fprintf(stderr, "Function calls not implemented yet\n");
			nobj = create_error_object(machine);
		}
	}
	return nobj;
}

