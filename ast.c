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
  return error_code;
}

int push_child(ast *node, ast child) {
  int error_code = 0;
  if ((node->children).length == (node->children).capacity)
    error_code = grow_ast_vector(&node->children);

  (node->children).data[(node->children).length++] = child;
  return error_code;
}

/*
 * AST Vector
 * =============
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
