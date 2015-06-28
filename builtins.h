#ifndef BUILTINS_H
#define BUILTINS_H

#include "scheme_forward.h"

struct Object *mcar(struct Machine *m, struct Object *args);
struct Object *mcdr(struct Machine *m, struct Object *args);
struct Object *mcadr(struct Machine *m, struct Object *args);
struct Object *cons(struct Machine *m, struct Object *args);
struct Object *meval(struct Machine *m, struct Object *args);
struct Object *quote(struct Machine *machine, struct Object *args);
struct Object *define(struct Machine *machine, struct Object *args);
struct Object *sum(struct Machine *machine, struct Object *args);
struct Object *prod(struct Machine *machine, struct Object *args);
struct Object *subtract(struct Machine *machine, struct Object *args);
struct Object *divide(struct Machine *machine, struct Object *args);

#endif
