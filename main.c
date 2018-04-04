#include <stdio.h>
#include <string.h>

#include "ringbuffer.h"
#include "lexer.h"

void print(lexeme *a) {
  printf("(lexeme) {class: %25s, content: %10s, line %2d, column %2d}\n",
	 lexeme_class_tostr(a->type),
	 a->type == LexStatementTerminator ? ".\\n" : a->content,
	 a->line, a->column);
}

int main() {
  
  ringbuffer b;
  init_stringbuffer(&b,
		    "1 + -x / 6^-y.\n"
		    "- 567.\n"
		    "true and not false xor true.\n"
		    "x < y.\n"
		    "x < y < z < w.\n"
		    "x < y < z and not w.\n"
		    "f(x, y).\n"
		    "A[i, j].\n"
		    "g(H[i, j], x mod n).\n");

  int error_code = 0;
  lexeme l;

  do {
    error_code = scan(&b, &l);
    print(&l);
  } while (l.type != LexEndOfFile);

  



  
  return 0;
}
