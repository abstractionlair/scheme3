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
};


struct Object *create_pair_object(struct Machine *machine, struct Object *car,
				struct Object *cdr);
struct Object *create_symbol_object(struct Machine *machine, struct String str);
struct Object *create_string_object(struct Machine *machine, struct String str);
struct Object *create_integer_object(struct Machine *machine, int integer);
struct Object *create_double_object(struct Machine *machine, double dbl);
bool obj_is_nil(struct Object * obj);
struct Machine *create_machine();
struct Object *eval(struct Machine *machine, struct Object *obj);

#endif
