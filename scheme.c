#include "base.h"
#include "builtins.h"
#include "env.h"
#include "eval.h"
#include "read.h"
#include "scheme.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

struct Object *create_builtin_func_object(struct Machine *machine,
					struct BuiltinFunc f)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeBuiltinFunc;
		obj->builtinFunc = f;
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
	case TypeBuiltinFunc:
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

bool machine_register_builtin_form(struct Machine *m, char *cname, builtinForm f)
{
	struct String name = string_from_cstring(cname);
	struct Object *symbol = create_symbol_object(m, name);
	struct BuiltinForm func = {.f = f};
	struct Object *funcObj = create_builtin_form_object(m, func);
	return env_update(&m->rootEnv->env, symbol->symbol, funcObj);
}

bool machine_register_builtin_func(struct Machine *m, char *cname, builtinFunc f)
{
	struct String name = string_from_cstring(cname);
	struct Object *symbol = create_symbol_object(m, name);
	struct BuiltinFunc func = {.f = f};
	struct Object *funcObj = create_builtin_func_object(m, func);
	return env_update(&m->rootEnv->env, symbol->symbol, funcObj);
}

struct Machine *create_machine()
{
	struct Machine *m = malloc(sizeof(*m));
	if (m) {
		m->symbols = make_string_array();
		m->rootEnv = create_env_object(m);
		m->env = m->rootEnv;

		machine_register_builtin_form(m, "define", define);
		machine_register_builtin_form(m, "quote", quote);

		machine_register_builtin_func(m, "eval", meval);
		machine_register_builtin_func(m, "car", mcar);
		machine_register_builtin_func(m, "cdr", mcdr);
		machine_register_builtin_func(m, "cadr", mcadr);
		machine_register_builtin_func(m, "+", sum);
		machine_register_builtin_func(m, "*", prod);
		machine_register_builtin_func(m, "-", subtract);
		machine_register_builtin_func(m, "/", divide);
	}
	return m;
}

