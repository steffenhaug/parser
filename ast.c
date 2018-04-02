#include <stdlib.h>

#include "ast.h"

/*
 * AST Node
 * ========
 */

void init_ast(ast *node, ast_class cls) {
  node->type = cls;
  node->value.i = 0;
  node->span = (ast_span) {0, 0, 0, 0};
  init_ast_vector(&node->children);
}

int free_ast(ast *node) {
  int error_code = free_ast_vector(&node->children);
  if (is_string_type(node->type))
    free(node->value.s);
  return error_code;
}

int push_child(ast *node, ast child) {
  int error_code = 0;
  if ((node->children).length == (node->children).capacity)
    error_code = grow_ast_vector(&node->children);

  (node->children).data[(node->children).length++] = child;
  return error_code;
}

/* ast_span
 * ========
 */

void set_span_start(ast *root, int line, int column) {
  root->span.start_line = line;
  root->span.start_column = column;
}

void set_span_end(ast *root, int line, int column) {
  root->span.end_line = line;
  root->span.end_column = column;
}


/*
 * ast_vector
 * ==========
 */

void init_ast_vector(ast_vector *nodes) {
  nodes->data = NULL;
  nodes->length = 0;
  nodes->capacity = 0;
}

int free_ast_vector(ast_vector *nodes) {
  if (nodes != NULL) {
    // free the nodes in the array individually...
    for (int i = 0 ; i < nodes->length; i++)
      free_ast(&nodes->data[i]);

    // then free the underlying array...
    free(nodes->data);

    // and zero all the members
    init_ast_vector(nodes);

    return 0;
  } else {
    return 1;
  }
}

int grow_ast_vector(ast_vector *nodes) {
  size_t new_capacity
    = nodes->capacity < AST_VECTOR_MIN_CAPACITY
    ? AST_VECTOR_MIN_CAPACITY
    : AST_VECTOR_GROWTH_FACTOR * nodes->capacity;

  ast* new_alloc = (ast*) realloc(nodes->data,
				  new_capacity * sizeof(*new_alloc));

  if (new_alloc != NULL) {
    nodes->capacity = new_capacity;
    nodes->data = new_alloc;
    return 0;
  } else {
    return 1;
  }
}

int fit_ast_vector(ast_vector *nodes) {
  ast* new_alloc = (ast*) realloc(nodes->data,
				  nodes->length * sizeof(*new_alloc));

  if (new_alloc != NULL) {
    nodes->capacity = nodes->length;
    nodes->data = new_alloc;
    return 0;
  } else {
    return 1;
  }
}


/* 
 * Printing
 * ======== */

#define becomes(sym, str) case sym: return str; break;
char *ast_class_tostr(ast_class cls) {
  switch (cls) {
    becomes(ASTPlus, "+");
    becomes(ASTMinus, "-");
    becomes(ASTUnaryMinus, "-");
    becomes(ASTMul, "*");
    becomes(ASTDiv, "/");
    becomes(ASTPow, "^");
    becomes(ASTMod, "mod");

    becomes(ASTAnd, "and");
    becomes(ASTXor, "xor");
    becomes(ASTOr, "or");
    becomes(ASTNot, "not");
    becomes(ASTComp, "cmp");

    becomes(ASTCall, "call");
    becomes(ASTSubscript, "subscript");
  default:
    return "<cannot represent>";
  }
}
#undef becomes

void print_sexpr(ast *tree) {
  switch (tree->type) {
    /* Root */
  case ASTRoot:
    for (int i = 0; i < tree->children.length; i++)
      print_sexpr(&tree->children.data[i]);
    printf("\n");
    break;
    /* Atoms */
  case ASTBool:
    printf("%s ", tree->value.b ? "true" : "false");
    break;
  case ASTInteger:
    printf("%lld ", tree->value.i);
    break;
  case ASTFloat:
    printf("%.2lf ", tree->value.d);
    break;
  case ASTString:
    printf("\"%s\" ", tree->value.s);
    break;
  case ASTIdentifier:
    printf("%s ", tree->value.s);
    break;
    /* Comparison Operators */
  case ASTLess:
    printf("< ");
    break;
  case ASTLessOrEqual:
    printf("<= ");
    break;
  case ASTGreater:
    printf("> ");
    break;
  case ASTGreaterOrEqual:
    printf(">= ");
    break;
  case ASTEquals:
    printf("== ");
    break;
  case ASTNotEqual:
    printf("!= ");
    break;
  case ASTCompOps:
  case ASTCompOperands:
    /* Print without ast class  */
    printf("(");
    for (int i = 0; i < tree->children.length; i++)
      print_sexpr(&tree->children.data[i]);
    printf("\b) ");
    break;
  default:
    /* Print the node as a list with ast class as the first element */
    printf("(%s ", ast_class_tostr(tree->type));
    for (int i = 0; i < tree->children.length; i++)
      print_sexpr(&tree->children.data[i]);
    printf("\b) ");
  }
}

bool is_string_type(ast_class cls) {
  return
    cls == ASTString ||
    cls == ASTIdentifier;
}
