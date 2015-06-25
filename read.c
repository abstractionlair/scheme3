#include "base.h"
#include "read.h"
#include "scheme.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char quote_escape(char c)
{
	switch (c) {
	case 'a':
		return '\a';
	case 'b':
		return '\b';
	case 'f':
		return '\f';
	case 'n':
		return '\n';
	case 'r':
		return '\r';
	case 't':
		return '\t';
	case 'v':
		return '\v';
	default:
		return c;
	}
}

bool is_filler(char c)
{
	return isspace(c);
}

bool is_self_delimited(char c)
{
	return c == ')' || c == '(';
}

bool is_delimiter(char c)
{
	return is_filler(c);
}

bool is_quote_start(char c)
{
	return c == '"';
}

bool is_quote_end(char c)
{
	return c == '"';
}

bool is_list_start(struct String word)
{
	/* NOTE: '\0' included in count. */
	return word.count == 2 && word.cstr[0] == '(';
}

bool is_list_end(struct String word)
{
	/* NOTE: '\0' included in count. */
	return word.count == 2 && word.cstr[0] == ')';
}

struct String read_word(FILE *stream)
{
	enum { normal, quote, quoteEscape } mode = normal;

	struct String str = make_string();
	while (1) {
		int c = getc(stream);
		if (c == EOF)
			goto out_str;
		switch (mode) {
		case normal:
			// Skip space at the beginning
			if (!str.count && is_filler(c))
				continue;
			// Ensure '(' and ')' form their own words.
			if (is_self_delimited(c)) {
				if (str.count)
					if (ungetc(c, stream) == EOF)
						goto out_no_str;
					else
						goto out_str;
				else {
					string_append(&str, c);;
					goto out_str;
				}
			}
			if (is_delimiter(c))
				goto out_str;
			string_append(&str, c);;
			if (is_quote_start(c))
				mode = quote;
			else if (is_delimiter(c))
				goto out_str;
			break;
		case quote:
			if ((char)c == '\\') {
				mode = quoteEscape;
			} else {
				string_append(&str, c);;
				if (is_quote_end(c))
					mode = normal;
			}
			break;
		case quoteEscape:
			string_append(&str, quote_escape(c));
			mode = quote;
			break;
		default:
			assert(0);
		}
	}
out_str:
	string_append(&str, '\0');
out_no_str:
	return str;
}


struct StringArray read_expression(FILE *stream)
{
	/*
	 * Reads text of a complete expression.
	 * Complete here means balanced parentheses.
	 */

	int countOpen = 0;
	int countClose = 0;
	struct StringArray words = make_string_array();

	while (true) {
		if (words.count && countOpen == countClose)
			return words;
		struct String nextWord = read_word(stream);
		if (!nextWord.cstr)
			return make_string_array();
		if (is_list_start(nextWord))
			++countOpen;
		else if (is_list_end(nextWord))
			++countClose;
		if (!string_array_append(&words, nextWord))
			return make_string_array();
	}
}

struct Object *read_list(struct Machine *machine, struct StringArray *words,
			ptrdiff_t *pos);

struct Object *read_non_list(struct Machine *machine, struct String word);

struct Object *read(struct Machine *machine, struct StringArray *words)
{
	/*
	 * Create a scheme object from a list of word strings.
	 * Takes ownership of the words and clears the array.
	 */

	if (!words->count) {
		free_string_array_shallow(words);
		return 0;
	}
	struct String word = words->strs[0];
	if (is_list_start(word)) {
		free(word.cstr);
		ptrdiff_t pos = 1;
		struct Object *obj = read_list(machine, words, &pos);
		free_string_array_shallow(words);
		return obj;
	}
	else {
		free_string_array_shallow(words);
		return read_non_list(machine, word);
	}
}

struct Object *read_list(struct Machine *machine, struct StringArray *words,
			ptrdiff_t *pos)
{
	struct Object *first = create_pair_object(machine, 0, 0);
	if (!first)
		return 0;
	struct Object *into = first;
	while (*pos < words->count) {
		struct String word = words->strs[*pos];
		++*pos;
		if (is_list_end(word)) {
			free(word.cstr);
			return first;
		} else if (is_list_start(word)) {
			free(word.cstr);
			into->pair.car = read_list(machine, words, pos);
		} else {
			into->pair.car = read_non_list(machine, word);
		}
		into->pair.cdr = create_pair_object(machine, 0, 0);
		if (!into->pair.cdr)
			return 0;
		into = into->pair.cdr;
	}
	assert(0);
	return 0;
}

enum Type deduce_type(struct String word)
{
	if (word.cstr[0] == '\"') {
		return TypeString;
	}
	else if (strchr("0123456789+-.", word.cstr[0])) {
		if (strpbrk(word.cstr, ".eE")) {
			return TypeDouble;
		} else {
			return TypeInteger;
		}
	}
	return TypeSymbol;
}

struct Object *read_non_list(struct Machine *machine, struct String word)
{
	size_t n;
	switch (deduce_type(word)) {
	case TypeSymbol:
		return create_symbol_object(machine, word);
	case TypeString:
		// Get rid of parentheses.
		n = strlen(word.cstr);
		memmove(word.cstr, word.cstr + 1, n - 2);
		word.cstr[n - 2] = '\0';
		return create_string_object(machine, word);
	case TypeInteger:
		return create_integer_object(machine, atoi(word.cstr));
	case TypeDouble:
		return create_double_object(machine, atof(word.cstr));
	default:
		assert(0);
	}
	return 0;
}
