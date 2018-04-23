#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define parser_error(message, ...)			     \
  fprintf(stderr,					     \
	  "Parser Error: " message "\n", ##__VA_ARGS__);

/*
 * Recursive Descent Functions
 * ===========================
 */

int parse_root(parser *p, ast *root) {
  int error_code = 0;
  int start_line = LA(p, 0)->line;
  int start_column = LA(p, 0)->column;

  init_ast(root, ASTRoot);

  while (LT(p, 0) != LexEndOfFile && !error_code) {
    ast stmt;
    error_code = parse_statement(p, &stmt);

    push_child(root, stmt);
  }

  set_span_start(root, start_line, start_column);
  set_span_end(root, LA(p, 0)->line, LA(p, 0)->column);
  return error_code;
}

int parse_statement(parser *p, ast *stmt) {
  int error_code = 0;
  int start_line = LA(p, 0)->line;
  int start_column = LA(p, 0)->column;
  
  // assume, for now, the statement is a simple expression
  parse_expression(p, stmt);
  match(p, LexStatementTerminator);

  set_span_start(stmt, start_line, start_column);
  set_span_end(stmt, LA(p, 0)->line, LA(p, 0)->column);
  return error_code;
}

int parse_identifier_list(parser *p, ast *node) {
  // This function ASSUMES THE NODE IS INITIALIZED!
  // This is pretty obvious, since the only things
  // that need identifier lists are function
  // definitions and things like that, which already
  // have stuff in it.
  // We don't initialize the node here, because we don't
  // want to reset it! So don't pass an unitiialized node.
  int error_code = 0;

  ast tmp;
  while (!error_code) {
    init_ast(&tmp, ASTIdentifier);
    error_code = match_store_value(p, LexIdentifier, &tmp);
    push_child(node, tmp);
    if (LT(p, 0) != LexComma)
      break;
    match(p, LexComma);
    if (is_closing_bracket(LT(p, 0)))
      break;
  }

  return error_code;
}

int parse_expression_list(parser *p, ast *node) {
  int error_code = 0;
  int start_line = LA(p, 0)->line;
  int start_column = LA(p, 0)->column;

  ast tmp;
  while (!error_code) {
    error_code = parse_expression(p, &tmp);
    push_child(node, tmp);
    if (LT(p, 0) != LexComma)
      break;
    match(p, LexComma);
    if (is_closing_bracket(LT(p, 0)))
      break;
  }

  return error_code;
}

/*
 * Operator Expressions
 * ====================
 */

int parse_par_expr(parser *p, ast *expr) {
  int error_code = 0;
  match(p, LexLeftParenthesis);
  error_code = parse_expression(p, expr);
  match(p, LexRightParenthesis);
  return error_code;
}

int parse_atom(parser *p, ast *atom) {
  int error_code = 0;
  switch(LT(p, 0)) {
  case LexLeftParenthesis:
    parse_par_expr(p, atom);
    break;
  case LexTrue:
    init_ast(atom, ASTBool);
    error_code = match_store_value(p, LexTrue, atom);
    break;
  case LexFalse:
    init_ast(atom, ASTBool);
    error_code = match_store_value(p, LexFalse, atom);
    break;
  case LexIdentifier:
    init_ast(atom, ASTIdentifier);
    error_code = match_store_value(p, LexIdentifier, atom);
    break;
  case LexDecInteger:
    init_ast(atom, ASTInteger);
    error_code = match_store_value(p, LexDecInteger, atom);
    break;
  case LexHexInteger:
    init_ast(atom, ASTInteger);
    error_code = match_store_value(p, LexHexInteger, atom);
    break;
  case LexBinInteger:
    init_ast(atom, ASTInteger);
    error_code = match_store_value(p, LexBinInteger, atom);
    break;
  case LexFloat:
    init_ast(atom, ASTFloat);
    error_code = match_store_value(p, LexFloat, atom);
    break;
  case LexString:
    init_ast(atom, ASTString);
    error_code = match_store_value(p, LexString, atom);
    break;
  default:
    parser_error("Expected atomic value, found %s. (line: %zu, column: %zu)",
		 lexeme_class_tostr(LT(p, 0)),
		 LA(p, 0)->line,
		 LA(p, 0)->column);
    error_code = EXPECTED_ATOM;
  }
  return error_code;
}

int parse_primary_expression(parser *p, ast *expr) {
  int error_code = 0;
  ast tmp;
  error_code = parse_atom(p, &tmp);
  // Parse "trailing bit":
  switch (LT(p, 0)) {
  case LexLeftParenthesis:
    // parse argument vector
    match(p, LexLeftParenthesis);
    init_ast(expr, ASTCall);
    push_child(expr, tmp);
    parse_expression_list(p, expr);
    match(p, LexRightParenthesis);
    break;
  case LexLeftSquareBracket:
    // parse subscript
    match(p, LexLeftSquareBracket);
    init_ast(expr, ASTSubscript);
    push_child(expr, tmp);
    parse_expression_list(p, expr);
    match(p, LexRightSquareBracket);
    break;
  case LexDot:
    // parse member indexing
    match(p, LexDot);
    init_ast(expr, ASTMember);
    push_child(expr, tmp);
    ast id;
    init_ast(&id, ASTIdentifier);
    match_store_value(p, LexIdentifier, &id);
    push_child(expr, id);
    break;
  default:
    *expr = tmp;
    break;
  }
  return error_code;
}

// mathematical
int parse_power(parser *p, ast *expr) {
  int error_code = 0;

  ast base, exponent;
  error_code = parse_primary_expression(p, &base);
  if (error_code)
    return error_code;

  if(LT(p, 0) == LexCaret) {
    match(p, LexCaret);
    error_code = parse_factor(p, &exponent);
    init_ast(expr, ASTPow);
    push_child(expr, base);
    push_child(expr, exponent);
  } else {
    *expr = base;
  }
  
  return error_code;
}

int parse_factor(parser *p, ast *expr) {
  int error_code = 0;

  if (LT(p, 0) == LexMinus) {
    match(p, LexMinus);
    ast operand;
    error_code = parse_factor(p, &operand);
    init_ast(expr, ASTUnaryMinus);
    push_child(expr, operand);
  } else {
    error_code = parse_power(p, expr);
  }
  
  return error_code;
}

int parse_term(parser *p, ast *expr) {
  int error_code = 0;

  ast left, right, tmp;
  error_code = parse_factor(p, &left);

  while (!error_code) {
    switch (LT(p, 0)) {
    case LexAsterisk:
      match(p, LexAsterisk);
      init_ast(&tmp, ASTMul);
      break;
    case LexSlash:
      match(p, LexSlash);
      init_ast(&tmp, ASTDiv);
      break;
    case LexMod:
      match(p, LexMod);
      init_ast(&tmp, ASTMod);
      break;
    default:
      goto exit_loop;
    }
    parse_factor(p, &right);
    push_child(&tmp, left);
    push_child(&tmp, right);
    left = tmp;
  }
 exit_loop:
  
  *expr = left;
  
  return error_code;
}

int parse_arith_expr(parser *p, ast *expr) {
  int error_code = 0;

  ast left, right, tmp;
  error_code = parse_term(p, &left);

  while (!error_code) {
    switch (LT(p, 0)) {
    case LexPlus:
      match(p, LexPlus);
      init_ast(&tmp, ASTPlus);
      break;
    case LexMinus:
      match(p, LexMinus);
      init_ast(&tmp, ASTMinus);
      break;
    default:
      goto exit_loop;
    }
    error_code = parse_term(p, &right);
    push_child(&tmp, left);
    push_child(&tmp, right);
    left = tmp;
  }
 exit_loop:
  
  *expr = left;
  return error_code;
}


int parse_comp_expr(parser *p, ast *expr) {
  int error_code = 0;

  ast left_op, operators, operands;
  error_code = parse_arith_expr(p, &left_op);

  // Return early with the supposed left operand if
  // the lexeme directly after is not actually a comparison.
  if (!is_comparison(LT(p, 0))) {
    *expr = left_op;
    return error_code;
  }
  
  init_ast(expr, ASTComp);
  init_ast(&operators, ASTCompOps);
  init_ast(&operands, ASTCompOperands);

  while (!error_code) {
    ast operator, operand;
    switch (LT(p, 0)) {
    case LexLessThan:
      match(p, LexLessThan);
      init_ast(&operator, ASTLess);
      break;
    case LexLessOrEq:
      match(p, LexLessOrEq);
      init_ast(&operator, ASTLessOrEqual);
      break;
    case LexGreaterThan:
      match(p, LexGreaterThan);
      init_ast(&operator, ASTGreater);
      break;
    case LexGreaterOrEq:
      match(p, LexGreaterOrEq);
      init_ast(&operator, ASTGreaterOrEqual);
      break;
    case LexDoubleEquals:
      match(p, LexDoubleEquals);
      init_ast(&operator, ASTEquals);
      break;
    case LexNotEqual:
      match(p, LexNotEqual);
      init_ast(&operator, ASTNotEqual);
      break;
    default:
      goto exit_loop;
    }

    error_code = parse_arith_expr(p, &operand);
    push_child(&operators, operator);
    push_child(&operands, operand);
  }
 exit_loop:

  push_child(expr, left_op);
  push_child(expr, operators);
  push_child(expr, operands);

  return error_code;
}

int parse_not_expr(parser *p, ast *expr) {
  int error_code = 0;

  if (LT(p, 0) == LexNot) {
    match(p, LexNot);
    ast operand;
    error_code = parse_not_expr(p, &operand);
    init_ast(expr, ASTNot);
    push_child(expr, operand);
  } else {
    error_code = parse_comp_expr(p, expr);
  }

  return error_code;
}

int parse_and_expr(parser *p, ast *expr) {
  int error_code = 0;

  ast left, right, tmp;
  error_code = parse_not_expr(p, &left);

  while (!error_code) {
    switch (LT(p, 0)) {
    case LexAnd:
      match(p, LexAnd);
      init_ast(&tmp, ASTAnd);
      break;
    default:
      goto exit_loop;
    }
    error_code = parse_not_expr(p, &right);
    push_child(&tmp, left);
    push_child(&tmp, right);
    left = tmp;
  }
 exit_loop:
  
  *expr = left;

  return error_code;
}

int parse_or_expr(parser *p, ast *expr) {
  int error_code = 0;

  ast left, right, tmp;
  error_code = parse_and_expr(p, &left);

  while (!error_code) {
    switch (LT(p, 0)) {
    case LexOr:
      match(p, LexOr);
      init_ast(&tmp, ASTOr);
      break;
    case LexXor:
      match(p, LexXor);
      init_ast(&tmp, ASTXor);
      break;
    default:
      goto exit_loop;
    }
    error_code = parse_and_expr(p, &right);
    push_child(&tmp, left);
    push_child(&tmp, right);
    left = tmp;
  }
 exit_loop:
  
  *expr = left;
  return error_code;
}


int parse_expression(parser *p, ast *expr) {
  int error_code = ok;
  // assume it is an arithmetic expression
  error_code = parse_or_expr(p, expr);
  return error_code;
}

int advance(parser *p) {
  int error_code = ok;

  // Free the previous lexeme
  free_lexeme(&p->lookahead[p->position]);

  // Get a new one in its place
  error_code = scan(p->input, &p->lookahead[p->position]);
  // We probably need to handle this.
  // If the scan yields an error, the lexeme will ne NULL (we just freed it),
  // and I would rather not do null checks inside the parser -- the murual
  // recursion is ugly enough as it is; we don't need to stray further from
  // the Backus-Naur form.

  // Advance the position, potentially wrapping around
  p->position = (p->position + 1) % MAX_LOOKAHEAD;
  return error_code;
}

int init_parser(parser *p, ringbuffer *b) {
  p->input = b;
  p->position = 0;
  
  // Fill the lookahead ring-buffer
  for (int i = 0; i < MAX_LOOKAHEAD; i++) {
    scan(p->input, &p->lookahead[i]);
  }

  return ok;
}

int free_parser(parser *p) {
  if (NULL == p)
    return warning(FREE_NULL);
  for (int i = 0; i < MAX_LOOKAHEAD; i++)
    free_lexeme(&p->lookahead[i]);
  return ok;
}

lexeme *LA(parser *p, size_t i) {
  return &p->lookahead[i + p->position];
}

lexeme_class LT(parser *p, size_t i) {
  return LA(p, i)->type;
}

int match(parser *p, lexeme_class cls) {
  if (LT(p, 0) != cls)
    goto failed_match;

  advance(p);
  return ok;

 failed_match:
  advance(p);
  parser_error("Failed to match %s, found %s. (line: %zu, column: %zu)",
	       lexeme_class_tostr(cls),
	       lexeme_class_tostr(LT(p, 0)),
	       LA(p, 0)->line,
	       LA(p, 0)->column);
  return error(MATCH_FAILED);
}

int match_store_value(parser *p, lexeme_class cls, ast *node) {
  if (LT(p, 0) != cls)
    goto failed_match;

  switch (cls) {
  case LexDecInteger:
    node->value.i = strtol(LA(p, 0)->content, NULL, 10);
    break;
  case LexHexInteger:
    // start reading after "0x"
    node->value.i = strtol(&LA(p, 0)->content[2], NULL, 16);
    break;
  case LexBinInteger:
    // start reading after "0b"
    node->value.i = strtol(&LA(p, 0)->content[2], NULL, 2);
    break;
  case LexFloat:
    node->value.d = atof(LA(p, 0)->content);
    break;
  case LexTrue:
    node->value.b = true;
    break;
  case LexFalse:
    node->value.b = false;
    break;
  case LexString:
  case LexIdentifier:
    // We "swap" the char pointers instead of mallocing a new string.
    // This leaves the lexeme with NULL as content, but this is okay
    // because it is immediately freed as we call advance.
    node->value.s = LA(p, 0)->content;
    LA(p, 0)->content = NULL;
    break;
  default:
    goto matched_lexeme_has_no_value;
  }

  advance(p);
  return ok;

 failed_match:
  advance(p);
  parser_error("Failed to match %s, found %s. (line: %zu, column: %zu)",
	       lexeme_class_tostr(cls),
	       lexeme_class_tostr(LT(p, 0)),
	       LA(p, 0)->line,
	       LA(p, 0)->column);
  return error(MATCH_FAILED);

 matched_lexeme_has_no_value:
  advance(p);
  parser_error("Expected lexeme with value. Found %s. (line: %zu, column: %zu)",
	       lexeme_class_tostr(LT(p, 0)),
	       LA(p, 0)->line,
	       LA(p, 0)->column);
  return error(MATCH_STORE_NO_VALUE);
}
