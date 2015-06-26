#include "base.h"
#include "env.h"
#include "read.h"
#include "scheme.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Object *alloc_object(struct Machine *machine)
{
	/*
	 * Centralize this so that later we can keep track of them and
	 * add garbage collection.
	 */
	return malloc(sizeof(struct Object));
}

struct Object *create_symbol_object(struct Machine *machine, struct String str)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeSymbol;
		obj->symbol = string_array_search(machine->symbols, str);
		if (obj->symbol == -1) {
			obj->symbol = machine->symbols.count;
			if (!string_array_append(&machine->symbols, str)) {
				free(obj);
				return 0;
			}
		}
	}
	return obj;
}

struct Object *create_string_object(struct Machine *machine, struct String str)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeString;
		obj->string = str;
	}
	return obj;
}

struct Object *create_integer_object(struct Machine *machine, int integer)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeInteger;
		obj->integer = integer;
	}
	return obj;
}

struct Object *create_double_object(struct Machine *machine, double dbl)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeDouble;
		obj->dbl = dbl;
	}
	return obj;
}

struct Object *create_pair_object(struct Machine *machine, struct Object *car,
				struct Object *cdr)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypePair;
		obj->pair.car = car;
		obj->pair.cdr = cdr;
	}
	return obj;
}

struct Object *create_error_object(struct Machine *machine)
{
	struct Object *obj = alloc_object(machine);
	if (obj)
		obj->type = TypeError;
	return obj;
}

struct Object *create_env_object(struct Machine *machine)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeEnv;
		obj->env = make_env();
	}
	return obj;
}

struct Object *create_builtin_form_object(struct Machine *machine,
					struct BuiltinForm f)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeBuiltinForm;
		obj->builtinForm = f;
	}
	return obj;
}

void destroy_object(struct Machine *machine, struct Object *obj)
{
	/* NOTE: we want to free things that are owned by the object.
	 * That does not include other objects referenced by the given object.
	 * That will eventually be handled by garbage collection.
	 */
	switch (obj->type) {
	case TypeString:
		free(obj->string.cstr);
		free(obj);
		return;
	case TypeSymbol:
	case TypeInteger:
	case TypeDouble:
	case TypePair:
	case TypeEnv:
	case TypeError:
	case TypeBuiltinForm:
		free(obj);
		return;
	}
	assert(0);
}

struct Object *car(struct Object *obj)
{
	return obj->pair.car;
}

struct Object *cdr(struct Object *obj)
{
	return obj->pair.cdr;
}

struct Object *cadr(struct Object *obj)
{
	return obj->pair.cdr->pair.car;
}

struct Object *reverse_list(struct Machine *machine, struct Object *inList)
{
	if (inList->type != TypePair)
		return inList;
	struct Object *outList = create_pair_object(machine, 0, 0);
	while (inList->type == TypePair) {
		outList = create_pair_object(machine, car(inList), outList);
		inList = inList->pair.cdr;
	}
	return outList;
}

bool obj_is_nil(struct Object * obj)
{
	return obj && obj->type == TypePair && !obj->pair.car && !obj->pair.cdr;
}

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

struct Object *define(struct Machine *machine, struct Object *args)
{
	if (args->type == TypePair) {
		Object *key = car(args);
		if (key->type != TypeSymbol) {
			fprintf(stderr,
				"The key for a define must be a symbol.\n");
			return create_error_object(machine);
		}
		if (cdr(args)->type != TypePair) {
			fprintf(stderr,
				"Don't understand second argument for define.\n");
			return create_error_object(machine);
		}
		Object *value = eval(machine, cadr(args));
		env_update(&machine->rootEnv->env, key->symbol, value);
		return create_pair_object(machine, 0, 0);
	}
	fprintf(stderr, "Don't know how to define what you asked for\n");
	return create_error_object(machine);
}

struct Object *quote(struct Machine *machine, struct Object *args)
{
	return car(args);
}

struct Machine *create_machine()
{
	struct Machine *m = malloc(sizeof(*m));
	if (m) {
		m->symbols = make_string_array();
		m->rootEnv = create_env_object(m);

		struct String name;
		struct Object *symbol;
		struct BuiltinForm func;
		struct Object *funcObj;

		/* define */
		name = string_from_cstring("define");
		symbol = create_symbol_object(m, name);
		func.f = define;
		funcObj = create_builtin_form_object(m, func);
		env_update(&m->rootEnv->env, symbol->symbol, funcObj);

		/* quote */
		name = string_from_cstring("quote");
		symbol = create_symbol_object(m, name);
		func.f = quote;
		funcObj = create_builtin_form_object(m, func);
		env_update(&m->rootEnv->env, symbol->symbol, funcObj);


	}
	return m;
}

