#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "lexer.h"

void test_lex_file(const char* f) {
  stream *s = stream_fromfile(f);
  lexeme *a;
  do {
    
    // get next lexeme
    a = scan(s);
    printf("lexeme {category: %3d, content: %10s, line %2d, column %2d}\n",
	   a->category, a->content, a->line, a->column);

    if (a->category == END_OF_FILE) {
      lexeme_delete(a);
      break;
    }
    lexeme_delete(a);
  } while (1);
  sclose(s);
}

void test_lex_str(const char* str) {
  stream *s = stream_fromstr(str);
  lexeme *a;
  do {
    
    // get next lexeme
    a = scan(s);
    printf("lexeme {category: %3d, content: %10s, line %2d, column %2d}\n",
	   a->category, a->content, a->line, a->column);
    if (a->category == END_OF_FILE) {
      lexeme_delete(a);
      break;
  
    }
    lexeme_delete(a);
  } while (1);
  sclose(s);
}

int main(int arc, const char *argv[]) {
  printf("lexed file\n");
  test_lex_file(argv[1]);
  printf("lexed string\n");
  test_lex_str("(->) [] {} 12.5e32 123\n 0x543FA");
  return 0;
}
