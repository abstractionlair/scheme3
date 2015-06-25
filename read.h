#ifndef READ_H
#define READ_H

#include <stdio.h>
#include "base.h"

struct StringArray read_expression(FILE *stream);
bool is_list_start(struct String word);
bool is_list_end(struct String word);

#endif
