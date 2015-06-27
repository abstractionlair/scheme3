#include "builtins.h"
#include "eval.h"
#include "scheme.h"
#include <assert.h>
#include <stdio.h>

struct Object *mcar(struct Machine *m, struct Object *args)
{
	struct Object *arg0 = car(args);
	return car(arg0);
}

struct Object *mcdr(struct Machine *m, struct Object *args)
{
	struct Object *arg0 = car(args);
	return cdr(arg0);
}

struct Object *mcadr(struct Machine *m, struct Object *args)
{
	struct Object *arg0 = car(args);
	return car(cdr(arg0));
}

struct Object *meval(struct Machine *m, struct Object *args)
{
	struct Object *arg0 = car(args);
	return eval(m, arg0);
}

struct Object *quote(struct Machine *machine, struct Object *args)
{
	struct Object *arg0 = car(args);
	return arg0;
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

struct Object *sum(struct Machine *machine, struct Object *args)
{
	struct Object *r = 0;
	int integer;
	double dbl;
	while (!obj_is_nil(args)) {
		struct Object *earg = car(args);
		switch (earg->type) {
		case TypeInteger:
			integer = earg->integer;
			if (!r) {
				r = create_integer_object(machine, integer);
			} else if (r->type == TypeInteger) {
				r->integer += integer;
			} else if (r->type == TypeDouble) {
				r->dbl += integer;
			} else {
				assert(0);
			}
			break;
		case TypeDouble:
			dbl = earg->dbl;
			if (!r) {
				r = create_double_object(machine, dbl);
			} else if (r->type == TypeInteger) {
				r->dbl = r->integer + dbl;
				r->type = TypeDouble;
			} else if (r->type == TypeDouble) {
				r->dbl += dbl;
			} else {
				assert(0);
			}
			break;
		default:
			assert(0);
		}
		args = cdr(args);
	}
	return r;
}

struct Object *prod(struct Machine *machine, struct Object *args)
{
	struct Object *r = 0;
	int integer;
	double dbl;
	while (!obj_is_nil(args)) {
		struct Object *earg = car(args);
		switch (earg->type) {
		case TypeInteger:
			integer = earg->integer;
			if (!r) {
				r = create_integer_object(machine, integer);
			} else if (r->type == TypeInteger) {
				r->integer *= integer;
			} else if (r->type == TypeDouble) {
				r->dbl *= integer;
			} else {
				assert(0);
			}
			break;
		case TypeDouble:
			dbl = earg->dbl;
			if (!r) {
				r = create_double_object(machine, dbl);
			} else if (r->type == TypeInteger) {
				r->dbl = r->integer * dbl ;
				r->type = TypeDouble;
			} else if (r->type == TypeDouble) {
				r->dbl *= dbl;
			} else {
				assert(0);
			}
			break;
		default:
			assert(0);
		}
		args = cdr(args);
	}
	return r;
}

struct Object *subtract(struct Machine *machine, struct Object *args)
{
	struct Object *r = 0;
	int integer;
	double dbl;
	int count = 0;
	while (!obj_is_nil(args)) {
		struct Object *earg = car(args);
		++count;
		switch (earg->type) {
		case TypeInteger:
			integer = earg->integer;
			if (!r) {
				r = create_integer_object(machine, integer);
			} else if (r->type == TypeInteger) {
				r->integer -= integer;
			} else if (r->type == TypeDouble) {
				r->dbl -= integer;
			} else {
				assert(0);
			}
			break;
		case TypeDouble:
			dbl = earg->dbl;
			if (!r) {
				r = create_double_object(machine, dbl);
			} else if (r->type == TypeInteger) {
				r->dbl = r->integer - dbl ;
				r->type = TypeDouble;
			} else if (r->type == TypeDouble) {
				r->dbl -= dbl;
			} else {
				assert(0);
			}
			break;
		default:
			assert(0);
		}
		args = cdr(args);
	}
	if (count == 1) {
		switch (r->type) {
		case TypeInteger:
			r->integer = -r->integer;
			break;
		case TypeDouble:
			r->dbl = -r->dbl;
			break;
		default:
			assert(0);
		}
	}
	return r;
}

struct Object *divide(struct Machine *machine, struct Object *args)
{
	struct Object *r = 0;
	int integer;
	double dbl;
	int count = 0;
	while (!obj_is_nil(args)) {
		struct Object *earg = car(args);		++count;
		switch (earg->type) {
		case TypeInteger:
			integer = earg->integer;
			if (!r) {
				r = create_integer_object(machine, integer);
			} else if (r->type == TypeInteger) {
				r->integer /= integer;
			} else if (r->type == TypeDouble) {
				r->dbl /= integer;
			} else {
				assert(0);
			}
			break;
		case TypeDouble:
			dbl = earg->dbl;
			if (!r) {
				r = create_double_object(machine, dbl);
			} else if (r->type == TypeInteger) {
				r->dbl = r->integer / dbl ;
				r->type = TypeDouble;
			} else if (r->type == TypeDouble) {
				r->dbl /= dbl;
			} else {
				assert(0);
			}
			break;
		default:
			assert(0);
		}
		args = cdr(args);
	}
	if (count == 1) {
		switch (r->type) {
		case TypeInteger:
			r->dbl = 1.0 / r->integer;
			r->type = TypeDouble;
			break;
		case TypeDouble:
			r->dbl = 1.0 / r->dbl;
			break;
		default:
			assert(0);
		}
	}
	return r;
}

