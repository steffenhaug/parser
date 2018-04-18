#include <stdio.h>
#include <string.h>

#include "ringbuffer.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"

void print(lexeme *a) {
  printf("(lexeme) {class: %25s, content: %10s, line %2zu, column %2zu}\n",
	 lexeme_class_tostr(a->type),
	 a->type == LexStatementTerminator ? ".\\n" : a->content,
	 a->line, a->column);
}

int main() {
  
  ringbuffer b;
  init_stringbuffer(&b,
		    "A[steffen.first_name + 3].\n");

  int error_code = 0;

  parser p;
  init_parser(&p, &b);
  
  ast tree;
  parse_root(&p, &tree);

  print_sexpr(&tree);
  


  

  free_ast(&tree);
  free_parser(&p);
  free_ringbuffer(&b);
  
  return 0;
}
