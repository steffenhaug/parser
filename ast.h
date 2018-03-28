#include <stdio.h>

#define AST_VECTOR_GROWTH_FACTOR 2
#define AST_VECTOR_MIN_CAPACITY 2

// ... X macro? :-)
typedef enum {
  ASTInteger,
  ASTFloat,
  ASTString,
  ASTRoot,
  ASTPlus,
  ASTMinus,
} ast_class;

typedef struct {
  unsigned int start_line, start_column, end_line, end_column;
} ast_span;

typedef struct {
  struct ast *data;
  size_t length;
  size_t capacity;
} ast_vector;

typedef union {
  long long i;
  double d;
  const char* s;
} ast_value;

typedef struct ast {
  ast_class type;
  ast_value value;
  ast_span span;
  ast_vector children;
} ast;

/*
 * ast (tree nodes)
 * ================
 */

void init_ast(ast* node, ast_class cls);

int free_ast(ast* node);

// this is just as mean as it sounds
int push_child(ast* node, ast child);

/*
 * ast_vector management
 * =====================
 */

void init_ast_vector(ast_vector *nodes);
/* Zeroes all the members of 'nodes'.
 * This includes pointing 'nodes.data' to NULL.
 * Does not malloc anything.
 */

int free_ast_vector(ast_vector *nodes);
/* Frees the underlying array if it is allocated.
 * Recursively frees all the nodes in the vector.
 * Zeroes all members.
 */

int grow_ast_vector(ast_vector *nodes);
/* Allocates memory for more elements.
 * If the capacity is lower the minimum-capacity, i.e.
 * in prectice if the vector is uninittialized, it allocates
 * space for this many elements. Otherwise the capacity is
 * doubled.
 */

int fit_ast_vector(ast_vector *nodes);
/* Shrinks the allocated memory to the current size, for
 * more compact storage.
 */
