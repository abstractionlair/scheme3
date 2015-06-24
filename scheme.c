#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

struct String {
	char *cstr;
	size_t count;
	size_t size;
};

struct StringArray {
	struct String *strs;
	size_t count;
	size_t size;
};

/* For circular references */
typedef struct Object Object;
typedef struct Pair Pair;
typedef struct Env Env;
typedef struct Machine Machine;

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

struct EnvEntry {
	ptrdiff_t key;
	struct Object *value;
};

struct Env {
	struct EnvEntry *map;
	size_t count;
	size_t size;
	struct Env *parent;
};

typedef struct Object *(*builtinForm)(struct Machine *machine,
				struct Object *args);

struct BuiltinForm {
	//struct Object *(*f)(struct Machine *machine, struct Object *args);
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
		struct BuiltinForm builtinForm;
	};
};

struct Machine {
	struct StringArray symbols;
	struct Env env;
};

struct String make_string(void)
{
	struct String s = {.cstr = 0, .count = 0, .size = 0};
	return s;
}

struct StringArray make_string_array(void)
{
	struct StringArray sa = {.strs = 0, .count = 0, .size = 0};
	return sa;
}

struct Env make_env(void)
{
	struct Env e = {.map = 0, .count = 0, .size = 0, .parent = 0};
	return e;
}

struct Machine make_machine(void)
{
	struct Machine m = {.symbols = make_string_array(), .env = make_env()};
	return m;
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

char quote_escape(char c)
{
	switch (c) {
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	default:
		return c;
	}
}

bool is_filler(char c)
{
	return isspace(c);
}

bool is_self_delimited(char c)
{
	return c == ')' || c == '(';
}

bool is_delimiter(char c)
{
	return is_filler(c);
}

bool is_quote_start(char c)
{
	return c == '"';
}

bool is_quote_end(char c)
{
	return c == '"';
}

bool is_list_start(struct String word)
{
	/* NOTE: '\0' included in count. */
	return word.count == 2 && word.cstr[0] == '(';
}

bool is_list_end(struct String word)
{
	/* NOTE: '\0' included in count. */
	return word.count == 2 && word.cstr[0] == ')';
}

bool string_append(struct String *str, char c)
{
	if (str->count >= str->size){
		size_t nsize = str->size + 32;
		char *ncstr = realloc(str->cstr, nsize);
		if (!ncstr)
			return false;
		str->cstr = ncstr;
		str->size = nsize;
	}
	(str->cstr)[(str->count)++] = c;
	return true;
}

void free_string(struct String *str)
{
	if (str->count)
		free(str->cstr);
	str->cstr = 0;
	str->count = 0;
	str->size = 0;
}

struct String string_from_cstring(char *cstr)
{
	struct String str = make_string();
	while (*cstr) {
		string_append(&str, *cstr);
		++cstr;
	}
	string_append(&str, *cstr); /* The Null */
	return str;
}
int string_compare(struct String s1, struct String s2)
{
	if (s1.count == 0 && s2.count == 0)
		return 0;
	else if (s1.count == 0)
		return -1;
	else if (s2.count == 0)
		return 1;
	else {
		char *cs1 = s1.cstr;
		char *cs2 = s2.cstr;
		char *end1 = s1.cstr + s1.count;
		char *end2 = s2.cstr + s2.count;
		while (cs1 != end1 && cs2 != end2) {
			int diff = (int)(*cs1) - (int)(*cs2);
			if (diff)
				return diff;
			++cs1;
			++cs2;
		}
		return s1.count - s2.count;
	}
}

bool string_array_append(struct StringArray *strs, struct String nstr)
{
	if (strs->count >= strs->size){
		size_t nsize = (strs->size + 16) * sizeof(struct String);
		struct String *nstrs = realloc(strs->strs, nsize);
		if (!nstrs)
			return false;
		strs->strs = nstrs;
		strs->size = nsize;
	}
	(strs->strs)[(strs->count)++] = nstr;
	return true;
}

void free_string_array(struct StringArray *stra)
{
	if (stra->count) {
		struct String *str = stra->strs;
		struct String *end = str + stra->count;
		for (; str != end; ++str)
			free_string(str);
		free(stra->strs);
	}
	stra->strs = 0;
	stra->count = 0;
	stra->size = 0;
}

ptrdiff_t string_array_search(struct StringArray stra, struct String str)
{
	for (ptrdiff_t i = 0; i != stra.count; ++i) {
		if (!string_compare(str, stra.strs[i]))
			return i;
	}
	return -1;
}
void free_string_array_shallow(struct StringArray *stra)
{
	/*
	 * For cases where ownership of the underlying strings has
	 * been moved.
	 */
	if (stra->count)
		free(stra->strs);
	stra->strs = 0;
	stra->count = 0;
	stra->size = 0;
}

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

struct String read_word(FILE *stream)
{
	enum { normal, quote, quoteEscape } mode = normal;

	struct String str = make_string();
	while (1) {
		int c = getc(stream);
		if (c == EOF)
			goto out_str;
		switch (mode) {
		case normal:
			// Skip space at the beginning
			if (!str.count && is_filler(c))
				continue;
			// Ensure '(' and ')' form their own words.
			if (is_self_delimited(c)) {
				if (str.count)
					if (ungetc(c, stream) == EOF)
						goto out_no_str;
					else
						goto out_str;
				else {
					string_append(&str, c);;
					goto out_str;
				}
			}
			if (is_delimiter(c))
				goto out_str;
			string_append(&str, c);;
			if (is_quote_start(c))
				mode = quote;
			else if (is_delimiter(c))
				goto out_str;
			break;
		case quote:
			if ((char)c == '\\') {
				mode = quoteEscape;
			} else {
				string_append(&str, c);;
				if (is_quote_end(c))
					mode = normal;
			}
			break;
		case quoteEscape:
			string_append(&str, quote_escape(c));
			mode = quote;
			break;
		default:
			assert(0);
		}
	}
out_str:
	string_append(&str, '\0');
out_no_str:
	return str;
}

struct StringArray read_expression(FILE *stream)
{
	/*
	 * Reads text of a complete expression.
	 * Complete here means balanced parentheses.
	 */

	int countOpen = 0;
	int countClose = 0;
	struct StringArray words = make_string_array();

	while (true) {
		if (words.count && countOpen == countClose)
			return words;
		struct String nextWord = read_word(stream);
		if (!nextWord.cstr)
			return make_string_array();
		if (is_list_start(nextWord))
			++countOpen;
		else if (is_list_end(nextWord))
			++countClose;
		if (!string_array_append(&words, nextWord))
			return make_string_array();
	}
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
		nobj = env_get(&machine->env, obj->symbol);
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

struct Object *print_args_addr(struct Machine *machine, struct Object *args)
{
	printf("You called me on %p\n", args);
	return create_pair_object(machine, 0, 0);
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
			fprintf(stderr, "The key for a define must be a symbol.\n");
			return create_pair_object(machine, 0, 0);
		}
		if (cdr(args)->type != TypePair) {
			fprintf(stderr, "Don't understand second argument for define.\n");
			return create_pair_object(machine, 0, 0);
		}
		Object *value = eval(machine, cadr(args));
		env_update(&machine->env, key->symbol, value);
		return create_pair_object(machine, 0, 0);
	}
	fprintf(stderr, "Don't know how to define what you asked for\n");
	return create_error_object(machine);
}

int main(int argc, char *argv[])
{
	struct Machine machine = make_machine();

	struct Object *s1 = create_symbol_object(&machine, string_from_cstring("s1"));
	struct Object *v1 = create_integer_object(&machine, 31);
	env_update(&machine.env, s1->symbol, v1);

	struct Object *s2 = create_symbol_object(&machine, string_from_cstring("paa"));
	struct BuiltinForm f2 = { .f = print_args_addr };
	struct Object *v2 = create_builtin_form_object(&machine, f2);
	env_update(&machine.env, s2->symbol, v2);

	struct Object *s3 = create_symbol_object(&machine, string_from_cstring("define"));
	struct BuiltinForm f3 = { .f = define };
	struct Object *v3 = create_builtin_form_object(&machine, f3);
	env_update(&machine.env, s3->symbol, v3);

	while (1) {
		struct StringArray words = read_expression(stdin);
		struct Object *obj = read(&machine, &words);

		obj_print(&machine, obj);
		printf("\n-> ");
		obj_print(&machine, eval(&machine, obj));
		printf("\n");
	}
	return 0;
}

/* int main(int argc, char *argv[]) */
/* { */
/* 	if (argc < 3) */
/* 		return -1; */
/* 	struct StringArray stra = make_string_array(); */
/* 	struct String key = string_from_cstring(argv[1]); */
/* 	for (ptrdiff_t i = 2; i != argc; ++i) */
/* 		string_array_append(&stra, string_from_cstring(argv[i])); */

/* 	printf("Key: %s\n", key.cstr); */
/* 	for (ptrdiff_t i = 0; i != stra.count; ++i) */
/* 		printf("%ti: %s\n", i, stra.strs[i].cstr); */

/* 	printf("%ti\n", string_array_search(stra, key)); */
/* 	return 0; */
/* } */
