#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "lexer.h"

void test_lex_str(const char* str) {
  printf("lexed string: ");
  printf("\"%s\", of length %lu\n", str, strlen(str));
  stream *s = stream_fromstr(str);
  lexeme *a;
  do {
    
    // get next lexeme
    a = scan(s);
    printf("(lexeme) {class: %20s, content: %10s, line %2d, column %2d}\n",
	   lexeme_class_tostr(a->type), a->content, a->line, a->column);
    if (a->type == EndOfFile) {
      free_lexeme(a);
      break;
  
    }
    free_lexeme(a);
  } while (1);
  sclose(s);
}

int main(int arc, const char *argv[]) {
  test_lex_str("\n\n"
	       "// squares the number x \n"
	       "func cube(x):           \n"
	       "  x^3.                \n\n");
  return 0;
}
