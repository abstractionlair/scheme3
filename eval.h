#ifndef EVAL_H
#define EVAL_H

#include "scheme_forward.h"

struct Object *eval(struct Machine *machine, struct Object *obj);

#endif
