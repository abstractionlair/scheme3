#include "eval.h"
#include "print.h"
#include "read.h"
#include "scheme.h"

int main(int argc, char *argv[])
{
	struct Machine *machine = create_machine();

	while (1) {
		struct StringArray words = read_expression(stdin);
		struct Object *obj = read(machine, &words);
		struct Object *nobj = eval(machine, obj);

		printf("main: ");
		obj_print(machine, obj);
		printf(" -> ");
		obj_print(machine, nobj);
		printf("\n");
	}
	return 0;
}
