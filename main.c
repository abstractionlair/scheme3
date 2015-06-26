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

		obj_print(machine, obj);
		printf("\n-> ");
		obj_print(machine, eval(machine, obj));
		printf("\n");
	}
	return 0;
}
