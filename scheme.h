#ifndef SCHEME_H
#define SCHEME_H

#include "base.h"
#include "env.h"
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
	struct Object *env;
};


struct Object *create_pair_object(struct Machine *machine, struct Object *car,
				struct Object *cdr);
struct Object *create_symbol_object(struct Machine *machine, struct String str);
struct Object *create_string_object(struct Machine *machine, struct String str);
struct Object *create_integer_object(struct Machine *machine, int integer);
struct Object *create_double_object(struct Machine *machine, double dbl);
struct Object *create_pair_object(struct Machine *machine, struct Object *car,
				struct Object *cdr);
struct Object *create_error_object(struct Machine *machine);
struct Object *create_env_object(struct Machine *machine);
struct Object *create_builtin_form_object(struct Machine *machine,
					struct BuiltinForm f);
void destroy_object(struct Machine *machine, struct Object *obj);
struct Object *car(struct Object *obj);
struct Object *cdr(struct Object *obj);
struct Object *cadr(struct Object *obj);
struct Object *reverse_list(struct Machine *machine, struct Object *inList);
bool obj_is_nil(struct Object * obj);
struct Machine *create_machine();

#endif
