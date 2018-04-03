#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "lexer.h"
#include "parser.h"

void test_parse_str(const char* str) {
  stream *s = stream_fromstr(str);
  parser p;
  init_parser(&p, s);

  ast tree;
  parse_root(&p, &tree);

  printf("%s ~>\n", str);
  print_sexpr(&tree);
  printf("start: %d, %d - end: %d, %d\n",
	 tree.span.start_line,
	 tree.span.start_column,
	 tree.span.end_line,
	 tree.span.end_column);
  printf("\n");

  free_ast(&tree);
  free_parser(&p);
  sclose(s);
}

int main(int arc, const char *argv[]) {
  printf("-- Parsing Some Sample Statements -- \n");
  test_parse_str("1 + -x / 6^-y.\n"
		 "- 567.\n"
		 "true and not false xor true.\n"
		 "x < y.\n"
		 "x < y < z < w.\n"
		 "x < y < z and not w.\n"
		 "f(x, y).\n"
		 "A[i, j].\n"
		 "g(H[i, j], x mod n).\n");
  return 0;
}
