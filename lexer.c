#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "lexer.h"

/* INTERNAL STACK MANAGEMENT */
#define push_next stack[++stackp] = sgetc(s);\
  charnum++
/* pushes the next character in the stream to
   the internal stack, and increments the
   column counter.
*/
#define pop_last sputc(stack[stackp], s);\
  charnum--;
/* pops the last character from the internal stack,
   and pushes it back into the stream. Decrements
   the column counter.
*/
#define return_lexeme(category) stack[stackp] = '\0';\
    return lexeme_new(category, stack, linenum, lexeme_start)
/* NULL TERMINATES THE STRING ON THE INTERNAL STACK!
   Then returns a pointer to a lexeme.
*/

lexeme *lexeme_new(int category, const char *content, int l, int c) {
  lexeme *t = malloc(sizeof(lexeme));
  t->category = category;
  t->line = l;
  t->column = c;
    
  int conlen = strlen(content) + 1;
  t->content = (char *) malloc(conlen);
  strcpy(t->content, content);
  return t;
}

void lexeme_delete(lexeme *l) {
  if (l->content != NULL)
    free(l->content);
  if (l != NULL)
    free(l);
}

// get the next lexeme
lexeme *scan(stream *s) {
  // stack to construct 
  char stack[LEXEME_STACK_DEPTH];
  int stackp;
  
  // line and column
  int lexeme_start;

  //
  // THIS NEEDS TO GO. Stream should manage its own state ENTIRELY by itself!
  //
  static int linenum = 1;
  static int charnum = 0;

 start:
  stackp = 0;
  stack[stackp] = sgetc(s);
  charnum++;
  lexeme_start = charnum;
  switch (stack[stackp]) {
  case LEXEME_WHITESPACE_NO_LINE_BREAK:
    goto start;
  case LEXEME_LINE_BREAK:
    linenum++;
    charnum = 0;
    goto start;
  case '0':
    goto seen_zero;
  case LEXEME_NONZERO_DIGIT:
    goto seen_digit;
  case '-':
    goto seen_minus;
  case EOF:
    charnum--;
    return lexeme_new(END_OF_FILE, "eof", linenum, lexeme_start);
  default:
    fprintf(stderr, "lexer.c ERROR: Lexer error, unexpected symbol \"%c\". (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 seen_zero:
  push_next;
  switch (stack[stackp]) {
  case '.':
    goto seen_decimal_point;
  case 'x':
  case 'X':
    goto seen_hex;
  case 'b':
  case 'B':
    goto seen_bin;
  case 'e':
  case 'E':
    goto scan_exp;
  default:
    pop_last;
    return_lexeme(INTEGER);
  }

 seen_digit:
  push_next;
  switch (stack[stackp]) {
  case LEXEME_DIGIT:
    goto seen_digit;
  case '.':
    goto seen_decimal_point;
  case 'e':
  case 'E':
    goto seen_exp;
  default:
    pop_last;
    return_lexeme(INTEGER);
  }

 seen_decimal_point:
  push_next;
  switch (stack[stackp]) {
  case LEXEME_DIGIT:
    goto seen_decimal_point;
  case 'e':
  case 'E':
    goto seen_exp;
  default:
    pop_last;
    return_lexeme(FLOAT);
  }

 seen_exp:
  push_next;
  switch (stack[stackp]) {
  case LEXEME_NONZERO_DIGIT:
    goto scan_exp;
  default:
    pop_last;
    // if the lexeme ends with "e" or "E"
    fprintf(stderr, "lexer.c ERROR: Lexer error, no exponent after \"e\" in number. Found \"%c\". (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 scan_exp:
  push_next;
  switch (stack[stackp]) {
  case LEXEME_DIGIT:
    goto scan_exp;
  default:
    pop_last;
    return_lexeme(FLOAT);
  }

 seen_hex:
  push_next;
  switch (stack[stackp]) {
  case LEXEME_HEX_DIGIT:
    goto scan_hex;
  default:
    pop_last;
    // if the lexeme ends with "x" or "X"
    fprintf(stderr, "lexer.c ERROR: Lexer error, no hex value after \"0x\". Found \"%c\". (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 scan_hex:
  push_next;
  switch (stack[stackp]) {
  case LEXEME_HEX_DIGIT:
    goto scan_hex;
  default:
    pop_last;
    return_lexeme(HEX_INTEGER);
  }

 seen_bin:
  push_next;
  switch (stack[stackp]) {
  case LEXEME_HEX_DIGIT:
    goto scan_bin;
  default:
    pop_last;
    // if the lexeme ends with "b" or "B"
    fprintf(stderr, "lexer.c ERROR: Lexer error, no binary value after \"0b\". Found \"%c\". (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }

 scan_bin:
  push_next;
  switch (stack[stackp]) {
  case '0': // we *reaaaally* need equivalence classes :-)
  case '1':
    goto scan_bin;
  default:
    pop_last;
    return_lexeme(BIN_INTEGER);
  }

 seen_minus:
  push_next;
  switch (stack[stackp]) {
  case '0':
    goto seen_zero;
  case LEXEME_NONZERO_DIGIT:
    goto seen_digit;
  case '>':
    return lexeme_new(RARROW, stack, linenum, lexeme_start);
  default:
    pop_last;
    fprintf(stderr, "lexer.c ERROR: parser error, unexpected %c (line %d, column %d)\n",
	    stack[stackp], linenum, charnum);
    exit(1);
  }
}
