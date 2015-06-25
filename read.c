#include "base.h"
#include "read.h"
#include <assert.h>
#include <ctype.h>

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
