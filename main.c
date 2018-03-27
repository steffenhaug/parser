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
  int line = 1;
  do {
    
    // get next lexeme
    a = scan(s);
    if (a->line > line) {
      printf("\n");
      line = a->line;

    }
    printf("(lexeme) %-20s %10s   (line %2d, column %2d)\n",
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
  test_lex_str("\nand or xor\n"
	       "func quicksort(L):                         \n"
	       "  cond:                                    \n"
	       "    empty?(L) or singleton?(L) then L,     \n"
	       "  otherwise:                               \n"
	       "    let v, vs  = uncons(L).                \n"
	       "        lower  = filter(fn x: x < v,  vs). \n"
	       "	higher = filter(fn x: x >= v, vs). \n"
	       "    in:                                    \n"
	       "      sort(lower) + [v] + sort(higher).    \n"
	       "\n\n");
  return 0;
}
