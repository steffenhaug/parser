#pragma once

#include <stddef.h>

#include "stdbool.h"
#include "ringbuffer.h"
#include "error.h"
#include "lexer.h"
#include "ast.h"

#define MAX_LOOKAHEAD 16

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

/*
 * Lookahead
 * =========
 */
lexeme *LA(parser *p, size_t i);
/* LA: ([L]ook[A]head)
 * Returs a pointer to the lexeme i steps
 * in front of the current lexeme.
 */

lexeme_class LT(parser *p, size_t i);
/* LT: ([L]ookahead [T]ype)
 * Returns the lexeme_class of the lexeme
 * i steps in front of the current lexeme.
 */

/*
 * Consuming Lexemes
 * =================
 */
int match(parser *p, lexeme_class cls);
/* Advances the parser one lexeme, returning an error
 * code if the lexeme does not correspond to the one provided.
 */

int match_store_value(parser *p, lexeme_class cls, ast *node);
/* Matches the provided lexeme (see 'match'), and stores the literal
 * value of the lexeme in the ast-node provided.
 */
