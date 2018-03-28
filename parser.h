#pragma once

#include <stddef.h>

#include "stdbool.h"
#include "stream.h"
#include "lexer.h"

#define MAX_LOOKAHEAD 8

/* Error handling
 * ==============
 * Defines error codes, and a macro for throwing errors.
 */

#define STREAM_IS_NULL 10
#define MATCH_FAILED 20

#define parser_error(message, ...)			     \
  fprintf(stderr,					     \
	  "Parser Error: " message "\n", ##__VA_ARGS__);


typedef struct {
  stream *input;
  size_t position;
  lexeme *lookahead[MAX_LOOKAHEAD];
} parser;

int init_parser(parser *p, stream *s);

int advance(parser *p);

/* Lookahead functions
 * -------------------
 * LA: ([L]ook[A]head)
 *   Returs a pointer to the lexeme i steps
 *   in front of the current lexeme.
 * LT: ([L]ookahead [T]ype)
 *   Returns the lexeme_class of the lexeme
 *   i steps in front of the current lexeme.
 * 
 * Note:
 * LA(p, 0) and LT(p, 0) gives the current
 * lexeme.
 */
lexeme *LA(parser *p, size_t i);
lexeme_class LT(parser *p, size_t i);

int match(parser *p, lexeme_class cls);
