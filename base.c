#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "base.h"

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
		size_t nsize = (strs->size + 16);
		size_t nbytes = nsize * sizeof(struct String);
		struct String *nstrs = realloc(strs->strs, nbytes);
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

char *strdup2(char *s1, char *s2)
{
	size_t l1 = strlen(s1);
	size_t l2 = strlen(s2);
	char *res = malloc(l1 + l2 + 1);
	strcpy(res, s1);
	strcpy(res + l1, s2);
	res[l1 + l2] = '\0';
	return res;
}

