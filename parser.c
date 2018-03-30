#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/*
 * Recursive Descent Functions
 * ===========================
 */

int parse_root(parser *p, ast *root) {
  int error_code = 0;
  init_ast(root, ASTRoot);

  set_span_start(root, LA(p, 0)->line, LA(p, 0)->column);
  while (LT(p, 0) != LexEndOfFile && !error_code) {
    ast stmt;
    error_code = parse_statement(p, &stmt);

    push_child(root, stmt);
  }
  set_span_end(root, LA(p, 0)->line, LA(p, 0)->column);
  return error_code;
}

int parse_statement(parser *p, ast *stmt) {
  int error_code = 0;
  // assume, for now, the statement is a simple expression
  parse_expression(p, stmt);
  match(p, LexDot);
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
    parser_error("Expected atomic value, found %s. (line: %d, column: %d)",
		 lexeme_class_tostr(LT(p, 0)),
		 LA(p, 0)->line,
		 LA(p, 0)->column);
    error_code = EXPECTED_ATOM;
  }
  return error_code;
}

int parse_call(parser *p, ast *call) {
  return -1; // not implemented
}

int parse_subscript(parser *p, ast *subsc) {
  return -1; // not implemented
}

int parse_primary_expr(parser *p, ast *expr) {
  int error_code = 0;
  switch (LT(p, 1)) {
  case LexLeftParenthesis:
    error_code = parse_call(p, expr);
    break;
  case LexLeftSquareBracket:
    error_code = parse_subscript(p, expr);
    break;
  default:
    error_code = parse_atom(p, expr);
  }
  return error_code;
}

// mathematical
int parse_power(parser *p, ast *expr) {
  int error_code = 0;

  ast base, exponent;
  error_code = parse_primary_expr(p, &base);
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
  // Or expressions are the "starting point" for
  // every single operatpr expression, because they
  // have the lowest prescedence.
  int error_code = 0;

  ast left, right, tmp;
  error_code = parse_not_expr(p, &left);

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
    error_code = parse_not_expr(p, &right);
    push_child(&tmp, left);
    push_child(&tmp, right);
    left = tmp;
  }
 exit_loop:
  
  *expr = left;
  return error_code;
}


int parse_expression(parser *p, ast *expr) {
  int error_code = 0;

  error_code = parse_or_expr(p, expr);
  return error_code;
}

/*
 * Parser Management
 * =================
 * advance, match, LA and LT are used to "encode the
 * grammar" in a recursive descent parser.
 */

int advance(parser *p) {
  // Free the previous lexeme
  free_lexeme(p->lookahead[p->position]);

  // Get a new one in its place
  p->lookahead[p->position] = scan(p->input);

  // Advance the position, potentially wrapping around
  p->position = (p->position + 1) % MAX_LOOKAHEAD;
  return 0;
}

int init_parser(parser *p, stream *s) {
  if (s == NULL) {
    parser_error("Failed to initialize Parser! Stream is NULL.");
    return STREAM_IS_NULL;
  }

  p->input = s;
  p->position = 0;
  
  // Fill the lookahead ring-buffer
  for (int i = 0; i < MAX_LOOKAHEAD; i++) {
    p->lookahead[i] = scan(p->input);
  }

  return 0;
}

lexeme *LA(parser *p, size_t i) {
  return p->lookahead[i + p->position];
}

lexeme_class LT(parser *p, size_t i) {
  return LA(p, i)->type;
}

int match(parser *p, lexeme_class cls) {
  // Assert that the current lexeme has the right type
  if (LT(p, 0) != cls) {
    parser_error("Failed to match %s, found %s. (line: %d, column: %d)",
		 lexeme_class_tostr(cls),
		 lexeme_class_tostr(LT(p, 0)),
		 LA(p, 0)->line,
		 LA(p, 0)->column);
    return MATCH_FAILED;
  }

  advance(p);
  return 0;
}

int match_store_value(parser *p, lexeme_class cls, ast *node) {
  if (LT(p, 0) != cls) {
    parser_error("Failed to match %s, found %s. (line: %d, column: %d)",
		 lexeme_class_tostr(cls),
		 lexeme_class_tostr(LT(p, 0)),
		 LA(p, 0)->line,
		 LA(p, 0)->column);
    return MATCH_FAILED;
  }

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
  case LexString: {
    char *s = (char*) malloc(strlen(LA(p, 0)->content) + 1);
    if (s == NULL) {
      parser_error("Failed do allocate memory for string!");
      return FAILED_MALLOC;
    }
    strcpy(s, LA(p, 0)->content);
    node->value.s = s;
    break;
  }
  case LexIdentifier: {
    char *s = (char*) malloc(strlen(LA(p, 0)->content) + 1);
    if (s == NULL) {
      parser_error("Failed do allocate memory for identifier!");
      return FAILED_MALLOC;
    }
    strcpy(s, LA(p, 0)->content);
    node->value.s = s;
    break;
  }
  default:
    parser_error("Expected lexeme with value. Found %s. (line: %d, column: %d)",
		 lexeme_class_tostr(LT(p, 0)),
		 LA(p, 0)->line,
		 LA(p, 0)->column);
    return MATCH_STORE_NO_VALUE;
  }

  advance(p);
  return 0;
}
