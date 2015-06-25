#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "env.h"
#include "base.h"
#include "read.h"
#include "scheme_forward.h"

enum Type {
	TypeSymbol,
	TypeString,
	TypeInteger,
	TypeDouble,
	TypePair,
	TypeEnv,
	//TypeFunction,
	//TypeForm,
	TypeBuiltinForm,
	TypeError
};

struct Pair {
	struct Object *car;
	struct Object *cdr;
};


typedef struct Object *(*builtinForm)(struct Machine *m, struct Object *args);

struct BuiltinForm {
	builtinForm f;
};

struct Object {
	enum Type type;
	union {
		ptrdiff_t symbol;
		struct String string;
		int integer;
		double dbl;
		struct Pair pair;
		struct Env env;
		struct BuiltinForm builtinForm;
	};
};

struct Machine {
	struct StringArray symbols;
	struct Object *rootEnv;
};




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

enum Type deduce_type(struct String word)
{
	if (word.cstr[0] == '\"') {
		return TypeString;
	}
	else if (strchr("0123456789+-.", word.cstr[0])) {
		if (strpbrk(word.cstr, ".eE")) {
			return TypeDouble;
		} else {
			return TypeInteger;
		}
	}
	return TypeSymbol;
}

struct Object *read_list(struct Machine *machine, struct StringArray *words,
			ptrdiff_t *pos);

struct Object *read_non_list(struct Machine *machine, struct String word);

struct Object *read(struct Machine *machine, struct StringArray *words)
{
	/*
	 * Create a scheme object from a list of word strings.
	 * Takes ownership of the words and clears the array.
	 */

	if (!words->count) {
		free_string_array_shallow(words);
		return 0;
	}
	struct String word = words->strs[0];
	if (is_list_start(word)) {
		free(word.cstr);
		ptrdiff_t pos = 1;
		struct Object *obj = read_list(machine, words, &pos);
		free_string_array_shallow(words);
		return obj;
	}
	else {
		free_string_array_shallow(words);
		return read_non_list(machine, word);
	}
}

struct Object *read_list(struct Machine *machine, struct StringArray *words,
			ptrdiff_t *pos)
{
	struct Object *first = create_pair_object(machine, 0, 0);
	if (!first)
		return 0;
	struct Object *into = first;
	while (*pos < words->count) {
		struct String word = words->strs[*pos];
		++*pos;
		if (is_list_end(word)) {
			free(word.cstr);
			return first;
		} else if (is_list_start(word)) {
			free(word.cstr);
			into->pair.car = read_list(machine, words, pos);
		} else {
			into->pair.car = read_non_list(machine, word);
		}
		into->pair.cdr = create_pair_object(machine, 0, 0);
		if (!into->pair.cdr)
			return 0;
		into = into->pair.cdr;
	}
	assert(0);
	return 0;
}

struct Object *read_non_list(struct Machine *machine, struct String word)
{
	size_t n;
	switch (deduce_type(word)) {
	case TypeSymbol:
		return create_symbol_object(machine, word);
	case TypeString:
		// Get rid of parentheses.
		n = strlen(word.cstr);
		memmove(word.cstr, word.cstr + 1, n - 2);
		word.cstr[n - 2] = '\0';
		return create_string_object(machine, word);
	case TypeInteger:
		return create_integer_object(machine, atoi(word.cstr));
	case TypeDouble:
		return create_double_object(machine, atof(word.cstr));
	default:
		assert(0);
	}
	return 0;
}

void obj_print_dotted(struct Machine *machine, struct Object *obj)
{
	switch (obj->type) {
	case TypeSymbol:
		printf("<%s>", machine->symbols.strs[obj->symbol].cstr);
		return;
	case TypeString:
		printf("\"%s\"", obj->string.cstr);
		return;
	case TypeInteger:
		printf("%i", obj->integer);
		return;
	case TypeDouble:
		printf("%f", obj->dbl);
		return;
	case TypePair:
		if (!obj->pair.car && !obj->pair.cdr)
			printf("nil");
		else {
			printf("(");
			if (!obj->pair.car)
				printf("null");
			else
				obj_print_dotted(machine, obj->pair.car);
			printf(". ");
			if (!obj->pair.cdr)
				printf("null");
			else
				obj_print_dotted(machine, obj->pair.cdr);
			printf(") ");
		}
		return;
	case TypeEnv:
		printf("*ENV*");
		return;
	case TypeError:
		printf("*ERROR*");
		return;
	case TypeBuiltinForm:
		printf("*BUILTIN_FORM*");
		return;
	}
}

void obj_print_inner(struct Machine *machine, struct Object *obj);

void obj_print(struct Machine *machine, struct Object *obj)
{
	if (obj->type == TypePair)
		printf("(");
	return obj_print_inner(machine, obj);
}

bool obj_is_nil(struct Object * obj)
{
	return obj && obj->type == TypePair && !obj->pair.car && !obj->pair.cdr;
}

void obj_print_inner(struct Machine *machine, struct Object *obj)
{
	switch (obj->type) {
	case TypeSymbol:
		printf("<%s> ", machine->symbols.strs[obj->symbol].cstr);
		return;
	case TypeString:
		printf("\"%s\" ", obj->string.cstr);
		return;
	case TypeInteger:
		printf("%i ", obj->integer);
		return;
	case TypeDouble:
		printf("%f ", obj->dbl);
		return;
	case TypePair:
		/* Car and Cdr are null */
		if (!obj->pair.car && !obj->pair.cdr) {
			printf("nil) ");
			return;
		}
		/* Only car is null -> unexpected so fallback to dotted. */
		if (!obj->pair.car) {
			obj_print_dotted(machine, obj);
			return;
		}
		if (obj->pair.car && obj->pair.car->type == TypePair)
			printf("( ");
		obj_print_inner(machine, obj->pair.car);

		// Cdr
		if (obj_is_nil(obj->pair.cdr))
			printf(") ");
		else
			obj_print_inner(machine, obj->pair.cdr);
		return;
	case TypeEnv:
		printf("*ENV*");
		return;
	case TypeError:
		printf("*ERROR*");
		return;
	case TypeBuiltinForm:
		printf("*BUILTIN_FORM*");
		return;
	}
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
		if (nobj)
			return nobj;
		else
			return create_error_object(machine);
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
			return create_pair_object(machine, 0, 0);
		}
		if (cdr(args)->type != TypePair) {
			fprintf(stderr,
				"Don't understand second argument for define.\n");
			return create_pair_object(machine, 0, 0);
		}
		Object *value = eval(machine, cadr(args));
		env_update(&machine->rootEnv->env, key->symbol, value);
		return create_pair_object(machine, 0, 0);
	}
	fprintf(stderr, "Don't know how to define what you asked for\n");
	return create_error_object(machine);
}

struct Machine *create_machine()
{
	struct Machine *machine = malloc(sizeof(*machine));
	if (machine) {
		machine->symbols = make_string_array();
		machine->rootEnv = create_env_object(machine);
		struct Object *s = create_symbol_object(machine,
							string_from_cstring("define"));
		struct BuiltinForm f = { .f = define };
		struct Object *v = create_builtin_form_object(machine, f);
		env_update(&machine->rootEnv->env, s->symbol, v);
	}
	return machine;
}

int main(int argc, char *argv[])
{
	struct Machine *machine = create_machine();

	while (1) {
		struct StringArray words = read_expression(stdin);
		struct Object *obj = read(machine, &words);

		obj_print(machine, obj);
		printf("\n-> ");
		obj_print(machine, eval(machine, obj));
		printf("\n");
	}
	return 0;
}
