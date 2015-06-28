#ifndef READ_H
#define READ_H

#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <stdio.h>
#include "base.h"
#include "scheme_forward.h"

typedef int (*getcFunc)(void *context);
typedef int (*ungetcFunc)(int c, void *context);

struct FileGetCharContext {
	FILE *stream;
};

int file_getc(void *context); // Use FileGetcContext
int file_ungetc(int c, void *context);  // Use FileGetcContext

struct ReadlineGetCharContext {
	char *line;
	size_t len;
	size_t pos;
	bool haveUnget;
	int unget;
	char *prompt;
};

void readline_init(void *context, char *prompt); // Use FileGetcContext
int readline_getc(void *context); // Use FileGetcContext
int readline_ungetc(int c, void *context);  // Use FileGetcContext




struct StringArray read_expression(getcFunc getcFunc, ungetcFunc ungetcFunc,
				void *context);
struct Object *read_scheme(struct Machine *machine,
			struct StringArray *words);


#endif
