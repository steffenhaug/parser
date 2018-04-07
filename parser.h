#pragma once

#include <stddef.h>

#include "stdbool.h"
#include "ringbuffer.h"
#include "lexer.h"
#include "ast.h"

#define MAX_LOOKAHEAD 8

/* Error handling
 * ==============
 * Defines error codes, and a macro for throwing errors.
 */

#define MATCH_FAILED 20
#define MATCH_STORE_NO_VALUE 21
#define EXPECTED_ATOM 30
#define FAILED_MALLOC 40

#define parser_error(message, ...)			     \
  fprintf(stderr,					     \
	  "Parser Error: " message "\n", ##__VA_ARGS__);


typedef struct {
  ringbuffer *input;
  size_t position;
  lexeme lookahead[MAX_LOOKAHEAD];
} parser;


int parse_root(parser *p, ast *root);
int parse_statement(parser *p, ast *stmt);

int parse_identifier_list(parser *p, ast *node);
int parse_expression_list(parser *p, ast *node);

int parse_par_expr(parser *p, ast *expr);
int parse_atom(parser *p, ast *atom);
int parse_primary_expression(parser *p, ast *expr);
int parse_power(parser *p, ast *expr);
int parse_factor(parser *p, ast *expr);
int parse_term(parser *p, ast *expr);
int parse_arith_expr(parser *p, ast *expr);
int parse_comp_expr(parser *p, ast *expr);
int parse_not_expr(parser *p, ast *expr);
int parse_and_expr(parser *p, ast *expr);
int parse_or_expr(parser *p, ast *expr);
int parse_expression(parser *p, ast *expr);

int init_parser(parser *p, ringbuffer *b);
int free_parser(parser *p);
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
int match_store_value(parser *p, lexeme_class cls, ast *node);
