#ifndef SCHEME_FORWARD_H
#define SCHEME_FORWARD_H

typedef struct Object Object;
typedef struct Pair Pair;
typedef struct Env Env;
typedef struct Machine Machine;
typedef struct Object *(*builtinForm)(struct Machine *m, struct Object *args);
typedef struct Object *(*builtinFunc)(struct Machine *m, struct Object *args);

#endif
