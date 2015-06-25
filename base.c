#include <stddef.h>
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

