#ifndef BASE_H
#define BASE_H

#include <stdbool.h>
#include <stddef.h>

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

struct String make_string(void);
struct StringArray make_string_array(void);
bool string_append(struct String *str, char c);
void free_string(struct String *str);
struct String string_from_cstring(char *cstr);
int string_compare(struct String s1, struct String s2);
bool string_array_append(struct StringArray *strs, struct String nstr);
void free_string_array(struct StringArray *stra);
ptrdiff_t string_array_search(struct StringArray stra, struct String str);
void free_string_array_shallow(struct StringArray *stra);
char *strdup2(char *s1, char *s2);

#endif
