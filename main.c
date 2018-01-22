#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"

#define LEXEME_STACK_DEPTH 256

struct lexeme {
  int category;
  char *content;
  int line;
  int column;
};

typedef struct lexeme lexeme;

lexeme *new_lexeme(int category, const char *content, int l, int c) {
  lexeme *t = malloc(sizeof(lexeme));
  t->category = category;
  t->line = l;
  t->column = c;
    
  int conlen = strlen(content) + 1;
  t->content = (char *) malloc(conlen);
  strcpy(t->content, content);
  return t;
}

void free_lexeme(lexeme *l) {
  free(l->content);
  free(l);
}

enum category {
  INTEGER,
  HEX_INTEGER,
  BIN_INTEGER,
  REAL,
  LPAREN,
  RPAREN,
  LCBRACKET,
  RCBRACKET,
  LSQBRACKET,
  RSQBRACKET,
  LARROW,
  RARROW,
  MINUS,
  END_OF_FILE,
};

enum state {
  START,
  SEEN_ZERO,
};


// get the next lexeme
lexeme *scan(stream *s) {
  int state = START;

  // stack to construct 
  char stack[LEXEME_STACK_DEPTH];
  int stackp;
  
  // line and column
  int lexeme_start;
  static int linenum = 1;
  static int charnum = 0;

 start:
  stackp = 0;
  stack[stackp] = sgetc(s);
  charnum++;
  lexeme_start = charnum;
  switch (stack[stackp]) {
  case ' ':
  case '\t':
    goto start;
  case '\n':
  case '\r':
    linenum++;
    charnum = 0;
    goto start;
  case '0':
    goto seen_zero;
  case '1': // this is messy, would be nice to test for equivalence classes...
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    goto seen_digit;
  case '-':
    goto seen_minus;
  case EOF:
    charnum--;
    return new_lexeme(END_OF_FILE, "eof", linenum, lexeme_start);
  }

 seen_zero:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '.':
    goto seen_decimal_point;
  case 'x':
  case 'X':
    goto scan_hex;
  case 'b':
  case 'B':
    goto scan_bin;
  case 'e':
  case 'E':
    goto scan_exp;
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    sputc(stack[stackp], s);
    charnum--;
    stack[stackp] = '\0';
    return new_lexeme(INTEGER, stack, linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 seen_digit:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    goto seen_digit;
  case '.':
    goto seen_decimal_point;
  case 'e':
  case 'E':
    goto seen_exp;
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    sputc(stack[stackp], s);
    charnum--;
    stack[stackp] = '\0';
    return new_lexeme(INTEGER, stack, linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 seen_decimal_point:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    goto seen_decimal_point;
  case 'e':
  case 'E':
    goto seen_exp;
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    sputc(stack[stackp], s);
    charnum--;
    stack[stackp] = '\0';
    return new_lexeme(REAL, stack, linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 seen_exp:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    goto scan_exp;
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 scan_exp:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    goto scan_exp;
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    sputc(stack[stackp], s);
    charnum--;
    stack[stackp] = '\0';
    return new_lexeme(REAL, stack, linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 scan_hex:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '0': // we *reaaaally* need equivalence classes :-)
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case 'a':
  case 'b':
  case 'c':
  case 'd':
  case 'e':
  case 'f':
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
    goto scan_hex;
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    sputc(stack[stackp], s);
    charnum--;
    stack[stackp] = '\0';
    return new_lexeme(HEX_INTEGER, stack, linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 scan_bin:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '0': // we *reaaaally* need equivalence classes :-)
  case '1':
    goto scan_bin;
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    sputc(stack[stackp], s);
    charnum--;
    stack[stackp] = '\0';
    return new_lexeme(BIN_INTEGER, stack, linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 seen_minus:
  stack[++stackp] = sgetc(s);
  charnum++;
  switch (stack[stackp]) {
  case '0':
    goto seen_zero;
  case '1': // this is messy, would be nice to test for equivalence classes...
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    goto seen_digit;
  case '>':
    return new_lexeme(RARROW, stack, linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

}

int main(int arc, const char *argv[]) {
  stream *s = sopen(argv[1]);

  lexeme *a;
  do {
    
    // get next lexeme
    a = scan(s);
    printf("lexeme {category: %3d, content: %10s, line %2d, column %2d}\n",
	   a->category, a->content, a->line, a->column);

    // free it
    if (a->category == END_OF_FILE) {
      free_lexeme(a);
      break;
    }
    free_lexeme(a);

  } while (1);
  
  
  sclose(s);
  return 0;
}
