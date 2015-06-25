#ifndef READ_H
#define READ_H

#include <stdio.h>
#include "base.h"
#include "scheme_forward.h"

struct StringArray read_expression(FILE *stream);
/* bool is_list_start(struct String word); */
/* bool is_list_end(struct String word); */
struct Object *read(struct Machine *machine, struct StringArray *words);
#endif
