#ifndef PRINT_H
#define PRINT_H

#include "scheme_forward.h"

void obj_print_dotted(struct Machine *machine, struct Object *obj);
void obj_print(struct Machine *machine, struct Object *obj);

#endif
