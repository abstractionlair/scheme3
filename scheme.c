#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>


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
	case 'r':		return '\r';
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

bool is_list_start(char *word)
{
	return word[0] == '(' && word[1] == '\0';
}

bool is_list_end(char *word)
{
	return word[0] == ')' && word[1] == '\0';
}

struct String {
	char * cstr;
	size_t count;
	size_t size;
};

/* bool string_append(char **str, size_t *count, size_t *size, char c) */
/* { */
/* 	if (*count >= *size){ */
/* 		char *nstr = realloc(*str, *count + 32); */
/* 		if (!nstr) */
/* 			return false; */
/* 		*str = nstr; */
/* 	} */
/* 	(*str)[(*count)++] = c; */
/* 	return true; */
/* } */

bool string_append(struct String *str, char c)
{
	if (str->count >= str->size){
		char *ncstr = realloc(str->cstr, str->count + 32);
		if (!ncstr)
			return false;
		str->cstr = ncstr;
	}
	(str->cstr)[(str->count)++] = c;
	return true;
}

bool string_array_append(char ***str, size_t *count, size_t *size, char *c)
{
	if (*count >= *size){
		char **nstr = realloc(*str, (*count + 16) * sizeof(char*));
		if (!nstr)
			return false;
		*str = nstr;
	}
	(*str)[(*count)++] = c;
	return true;
}

char *read_word(FILE *stream)
{
	enum { normal, quote, quoteEscape } mode = normal;
	
	//char * str = 0;
	//size_t count = 0;
	//size_t size = 0;
	struct String str = { .cstr = 0, .count = 0, .size = 0 };
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
					//string_append(&str, &count, &size, c);;
					string_append(&str, c);;
					goto out_str;
				}
			}
			if (is_delimiter(c))
				goto out_str;
			//string_append(&str, &count, &size, c);;
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
				//string_append(&str, &count, &size, c);;
				string_append(&str, c);;
				if (is_quote_end(c))
					mode = normal;
			}
			break;
		case quoteEscape:
			//string_append(&str, &count, &size, quote_escape(c));
			string_append(&str, quote_escape(c));
			mode = quote;
			break;
		default:
			assert(0);
		}
	}
out_str:
	//string_append(&str, &count, &size, '\0');
	string_append(&str, '\0');
out_no_str:
	return str.cstr;
}

char **read_expression(size_t *countWords, size_t *sizeWords, FILE *stream)
{
	/*
	 * Reads text of a complete expression.
	 * Complete here means balanced parentheses.
	 */

	int countOpen = 0;
	int countClose = 0;
	char **words = 0;
	
	*countWords = 0;
	*sizeWords = 0;
	while (true) {
		if (*countWords && countOpen == countClose)
			return words;
		char *nextWord = read_word(stream);
		if (!nextWord)
			return 0;
		if (is_list_start(nextWord))
			++countOpen;
		else if (is_list_end(nextWord))
			++countClose;
		if (!string_array_append(&words, countWords, sizeWords, nextWord))
			return 0;
	}
}

// For circular references
typedef struct Object Object; 
typedef struct Pair Pair;

struct Machine {
};

struct Pair {
	struct Object *car;
	struct Object *cdr;
};

enum Type { 
	TypeSymbol,
	TypeString,
	TypeInteger,
	TypeDouble,
	TypePair,
	TypeError
};

struct Object {
	enum Type type;
	union {
		char const *symbol;
		char const *string;
		int integer;
		double dbl;
		struct Pair pair;
	};
};

struct Object *alloc_object(struct Machine *machine)
{
	// Centralize this so that later we can keep track of them.
	return malloc(sizeof(struct Object));
}

struct Object *create_symbol_object(struct Machine *machine, char *str)
{
	struct Object *obj = alloc_object(machine);
	if (obj) {
		obj->type = TypeSymbol;
		obj->symbol = str;
	}
	return obj;
}

struct Object *create_string_object(struct Machine *machine, char *str)
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

void destroy_object(struct Machine *machine, struct Object *obj)
{
	/* NOTE: we want to free things that are owned by the object.
	 * That does not include other objects referenced by the given object.
	 * That will eventually be handled by garbage collection.
	 */
	switch (obj->type) {
	case TypeString:
		free((void*)obj->string);
		free(obj);
		return;
	case TypePair:
	case TypeError:
		free(obj);
		return;
	default:
		assert(0);
	}
}

struct Object *reverse_list(struct Machine *machine, struct Object *inList)
{
	if (inList->type != TypePair)
		return inList;
	struct Object *outList = create_pair_object(machine, 0, 0);
	while (inList->type == TypePair) {
		outList = create_pair_object(machine, inList->pair.car, outList);
		inList = inList->pair.cdr;
	}
	return outList;
}

struct Object * read_list(struct Machine *machine, char **words, size_t count,
			size_t size, size_t *pos);

struct Object * read_non_list(struct Machine *machine, char *word);

struct Object * read(struct Machine *machine, char **words, size_t count,
		size_t size, size_t *pos)
{
	/*
	 * As pos is incremented, the words are consumed.  The caller
	 * should forget about them and certainly not free them.  Not all of
	 * them need be consumed in a given call.
	 */
	if (*pos >= count)
		return 0;
	char *word = words[*pos];
	++*pos;
	if (is_list_start(word)) {
		free(word);
		return read_list(machine, words, count, size, pos);
	}
	else {
		return read_non_list(machine, word);
	}
}

struct Object * read_list(struct Machine *machine, char **words, size_t count,
			size_t size, size_t *pos)
{
	if (*pos >= count)
		return 0;
	struct Object *first = create_pair_object(machine, 0, 0);
	if (!first)
		return 0;
	struct Object *into = first;
	while (*pos < count) {
		char *word = words[*pos];
		++*pos;
		if (is_list_end(word)) {
			free(word);
			return first;
		} else if (is_list_start(word)) {
			free(word);
			into->pair.car = read_list(machine, words, count, size, pos);
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

enum Type deduce_type(char *word)
{
	if (word[0] == '\"') {
		return TypeString;
	}
	else if (strchr("0123456789+-.", word[0])) {
		if (strpbrk(word, ".eE")) {
			return TypeDouble;
		} else {
			return TypeInteger;
		}
	}
	return TypeSymbol;
}

struct Object *read_non_list(struct Machine *machine, char *word)
{
	size_t n;
	switch (deduce_type(word)) {
	case TypeSymbol:
		return create_symbol_object(machine, word);
	case TypeString:
		// Get rid of parentheses.
		n = strlen(word);
		memmove(word, word + 1, n - 2);
		word[n - 2] = '\0';
		return create_string_object(machine, word);
	case TypeInteger:
		return create_integer_object(machine, atoi(word));
	case TypeDouble:
		return create_double_object(machine, atof(word));
	default:
		assert(0);
	}
	return 0;
}

void obj_print_dotted(struct Machine *machine, struct Object *obj)
{
	switch (obj->type) {
	case TypeSymbol:
		printf("<%s>", obj->symbol);
		return;
	case TypeString:
		printf("\"%s\"", obj->string);
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
	case TypeError:
		printf("*ERROR*");
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
		printf("<%s> ", obj->symbol);
		return;
	case TypeString:
		printf("\"%s\" ", obj->string);
		return;
	case TypeInteger:
		printf("%i ", obj->integer);
		return;
	case TypeDouble:
		printf("%f ", obj->dbl);
		return;
	case TypePair:
		// Car and Cdr are null
		if (!obj->pair.car && !obj->pair.cdr) {
			printf("nil) ");
			return;
		}
		// Only car is null -> unexpected so fallback to dotted.
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
	case TypeError:
		printf("*ERROR*");
		return;
	}
}

struct Object *eval(struct Machine *machine, struct Object *obj)
{
	switch (obj->type) {
	case TypeSymbol:
	case TypeString:
	case TypeInteger:
	case TypeDouble:
	case TypeError:
		return obj;
	case TypePair:
		fprintf(stderr, "Function calls not implemented yet\n");
		return create_error_object(machine);
	}
	return 0;
}




 
/* int main(int argc, char *argv[]) */
/* { */
/* 	while (1) { */
/* 		char * str = read_word(stdin); */
/* 		printf("%s\n", str); */
/* 		free(str); */
/* 	} */
/* 	return 0; */
/* } */

/* int main(int argc, char *argv[]) */
/* { */
/* 	while (1) { */
/* 		size_t countWords; */
/* 		size_t sizeWords; */
/* 		char **words = read_expression(&countWords, &sizeWords, stdin); */
/* 		for (size_t i = 0; i != countWords; ++i) { */
/* 			printf("%s ", words[i]); */
/* 			free(words[i]); */
/* 		} */
/* 		printf("\n"); */
/* 		free(words); */
/* 	} */
/* 	return 0; */
/* } */

int main(int argc, char *argv[])
{
	struct Machine machine;
	while (1) {
		size_t countWords;
		size_t sizeWords;
		char **words = read_expression(&countWords, &sizeWords, stdin);
		size_t position = 0;
		struct Object *obj = read(&machine, words, countWords, sizeWords, &position);
		//obj_print_dotted(&machine, obj);
		//printf("\n");
		obj_print(&machine, obj);
		printf("\n-> ");
		obj_print(&machine, eval(&machine, obj));
		printf("\n");
		free(words);
	}
	return 0;
}
