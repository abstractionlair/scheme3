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

struct Object *eval_closure(struct Machine *m, struct Object *closure,
			struct Object *argVals)
{
	// Create new env, with parent captured when the closure was defined
	struct Object * newEnv = create_env_object(m);
	newEnv->env.parent = closure->closure.env;

	// Populate the new environment with the args passed in
	struct Object *argDefs = closure->closure.args;
	while (!obj_is_nil(argDefs) && !obj_is_nil(argVals)) {
		struct Object *key = car(argDefs);
		struct Object *val = car(argVals);
		env_update(&newEnv->env, key->symbol, val);
		argDefs = cdr(argDefs);
		argVals = cdr(argDefs);
	}

	// Push new env
	struct Object * oldEnv = m->env;
	m->env = newEnv;

	struct Object *res = eval(m, closure->closure.body);

	// Pop new env
	m->env = oldEnv;

	return res;
}

struct Object *eval_pair(struct Machine *machine, struct Object *obj)
{
	if (obj->pair.car) {
		struct Object *ecar = eval(machine, obj->pair.car);
		struct Object *eargs;
		switch (ecar->type) {
		case TypeBuiltinForm:
			return ecar->builtinForm.f(machine, obj->pair.cdr);
		case TypeBuiltinFunc:
			eargs = eval_list_items(machine, obj->pair.cdr);
			return ecar->builtinFunc.f(machine, eargs);
		case TypeClosure:
			eargs = eval_list_items(machine, obj->pair.cdr);
			return eval_closure(machine, ecar, eargs);
		case TypeSymbol:
		case TypeString:
		case TypeInteger:
		case TypeDouble:
		case TypePair:
		case TypeEnv:
		case TypeError:
			fprintf(stderr, "The first element isn't something executable\n");
			return create_error_object(machine);
		}
	}
	fprintf(stderr, "eval_pair: car is null.\n");
	return create_error_object(machine);
}

