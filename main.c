#include "eval.h"
#include "print.h"
#include "read.h"
#include "scheme.h"

int main(int argc, char *argv[])
{
	struct Machine *machine = create_machine();
	//struct FileGetCharContext getcContext = {.stream = stdin};
	struct ReadlineGetCharContext readlineContext;
	readline_init(&readlineContext, "> ");

	while (1) {
		/* struct StringArray words = read_expression(file_getc, */
		/* 					file_ungetc, */
		/* 					&getcContext); */
		struct StringArray words = read_expression(readline_getc,
							readline_ungetc,
							&readlineContext);
		struct Object *obj = read_scheme(machine, &words);
		struct Object *nobj = eval(machine, obj);
		obj_print(machine, nobj);
		printf("\n\n");
	}
	return 0;
}
