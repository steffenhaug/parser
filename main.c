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
  printf("\n");

  free_ast(&tree);
  free_parser(&p);
  sclose(s);
}

int main(int arc, const char *argv[]) {
  printf("-- Parsing Some Sample Statements -- \n");
  test_parse_str("1 + -x / 6^-y.\n");
  test_parse_str("1 - 5.\n");
  test_parse_str("true and not false xor true.\n");
  test_parse_str("x < y.\n");
  test_parse_str("x < y < z < w.\n");
  test_parse_str("x < y < z and not w.\n");
  test_parse_str("f(x, y).\n");
  test_parse_str("A[i, j].\n");
  test_parse_str("g(H[i, j], x mod n).\n");
  return 0;
}
