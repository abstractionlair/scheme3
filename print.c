#include "print.h"
#include "scheme.h"
#include <stdio.h>

void obj_print_dotted(struct Machine *machine, struct Object *obj)
{
	if (!obj) {
		printf("null");
	} else {
		switch (obj->type) {
		case TypeSymbol:
			printf("<%s>", machine->symbols.strs[obj->symbol].cstr);
			return;
		case TypeString:
			printf("\"%s\"", obj->string.cstr);
			return;
		case TypeInteger:
			printf("%i", obj->integer);
			return;
		case TypeDouble:
			printf("%f", obj->dbl);
			return;
		case TypePair:
			if (obj->pair.car && !obj->pair.cdr) {
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
		case TypeEnv:
			printf("*ENV*");
			return;
		case TypeError:
			printf("*ERROR*");
			return;
		case TypeBuiltinForm:
			printf("*BUILTIN_FORM*");
			return;
		case TypeBuiltinFunc:
			printf("*BUILTIN_FUNC*");
			return;
		case TypeClosure:
			printf("*CLOSURE*");
			return;
		}
	}
}

void obj_print_inner(struct Machine *machine, struct Object *obj);

void obj_print(struct Machine *machine, struct Object *obj)
{
	if (!obj) {
		printf("null");
	} else if (obj->type == TypePair) {
		printf("(");
	}
	return obj_print_inner(machine, obj);
}

void obj_print_inner(struct Machine *machine, struct Object *obj)
{
	if (!obj) {
		printf("null");
	} else {
		switch (obj->type) {
		case TypeSymbol:
			printf("<%s> ", machine->symbols.strs[obj->symbol].cstr);
			return;
		case TypeString:
			printf("\"%s\" ", obj->string.cstr);
			return;
		case TypeInteger:
			printf("%i ", obj->integer);
			return;
		case TypeDouble:
			printf("%f ", obj->dbl);
			return;
		case TypePair:
			/* Car and Cdr are null */
			//if (!obj->pair.car && !obj->pair.cdr) {
			if (obj_is_nil(obj)) {
				printf(") ");
				return;
			}
			if (!obj->pair.car) {
				/* Unexpected. Fallback to dotted. */
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
		case TypeEnv:
			printf("*ENV*");
			return;
		case TypeError:
			printf("*ERROR*");
			return;
		case TypeBuiltinForm:
			printf("*BUILTIN_FORM*");
			return;
		case TypeBuiltinFunc:
			printf("*BUILTIN_FUNC*");
			return;
		case TypeClosure:
			printf("*CLOSURE*");
			return;
		}
	}
}

