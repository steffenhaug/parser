#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"
#include "lexer.h"

// Note that this is distinct from lexer_error, which is specifically
// for errors in the lexer state machine.
#define lexeme_error(message, ...)			       \
  fprintf(stderr,					       \
	  "Lexeme Error: " message "\n",		       \
	  ##__VA_ARGS__);	       

int init_lexeme(lexeme *l, lexeme_class cls, const char *content,
		size_t line, size_t column) {
  l->type = cls;
  l->line = line;
  l->column = column;

  if (content == NULL) {
    l->content = NULL;
  } else {
    int size_of_content = strlen(content) + 1;
    l->content = (char *) malloc(size_of_content);
    if (l->content == NULL)
      goto failed_malloc;
    strcpy(l->content, content);
  }

  return ok;

 failed_malloc:
  lexeme_error("Failed to allocate memory for content! (content = %s on line %zu, col %zu)",
	       content, line, column);
  return error(FAILED_MALLOC_CONTENT);
}

int free_lexeme(lexeme *l) {
  if (l == NULL)
    return warning(FREE_NULL);

  free(l->content);
  l->line = 0;
  l->column = 0;
  l->content = NULL;

  return ok;
}

lexeme_class id_or_keyword(const char *contents) {
  /* Returns the lexeme class corresponding to the string.
   * If the string does not match a keyword, the identifier class
   * is returned. 
   */
#define check(str, id) if (strcmp(contents, str) == 0) return id
  check("mod", LexMod);
  check("func", LexFunc);
  check("fn", LexFn);
  check("use", LexUse);
  check("as", LexAs);
  check("let", LexLet);
  check("if", LexIf);
  check("else", LexElse);
  check("cases", LexCases);
  check("otherwise", LexOtherwise);
  check("not", LexNot);
  check("and", LexAnd);
  check("or", LexOr);
  check("xor", LexXor);
  check("true", LexTrue);
  check("false", LexFalse);
#undef check
  return LexIdentifier;
}

int scan(ringbuffer *input, lexeme *l) {
#define lexer_error(message, ...)			       \
  fprintf(stderr,					       \
	  "Lexer Error: " message " (line %ld, column %ld)\n", \
	  ##__VA_ARGS__, input->line, input->column);	       

#define munch get_character(input, &munched[++stackp])
// Pull a new character from the input stream.

#define drop stackp--
// Drop the character on the top of the stack.

#define push(element) munched[++stackp] = element
// Push an element, not necessarily from the input stream, to the stack.

#define last_munched munched[stackp]
// Top of the 'munched' stack.

#define yield(category)						\
  init_lexeme(l, category, NULL, input->line, munch_start);	\
  return ok
// Initializes the lexeme just scanned.

#define yield_value(category)					\
  munched[stackp + 1] = '\0';					\
  init_lexeme(l, category, munched, input->line, munch_start);	\
  return ok
// Initializes the lexeme just scanned, and *copies* 'munched' to its content.

  char munched[LEXEME_STACK_DEPTH];
  int stackp;
  int munch_start;
  /* munched:
   *   A stack containing the characters that have
   *   been read from the stream since the last time
   *   it entered the start-state.
   *
   * stackp:
   *   The index of the top of the stack.
   * 
   * munch_start:
   *   A record of the column (in the input stream)
   *   the current lexeme started. It is not necessary
   *   to record the start line, because a lexeme can
   *   not span multiple lines.
   */

 start:
  // prepare the stack
  stackp = 0;
  get_character(input, &munched[stackp]);
  // record the start column in the input
  munch_start = input->column;
  switch (last_munched) {
  case WHITESPACE:
    goto start;
  case '0':
    goto seen_zero;
  case NONZERO_DIGIT:
    goto seen_digit;
  case '\"':
    drop; // get rid of the quote we just ate
    goto scan_string;
  case ALPHA: // (a-z,A-Z,_) are the only valid start of identifiers
  case '_':
    goto scan_identifier;
  case EOF:
    yield(LexEndOfFile);
  case '(':
    yield(LexLeftParenthesis);
  case ')':
    yield(LexRightParenthesis);
  case '[':
    yield(LexLeftSquareBracket);
  case ']':
    yield(LexRightSquareBracket);
  case '{':
    yield(LexLeftCurlyBrace);
  case '}':
    yield(LexRightCurlyBrace);
  case '<':
    goto seen_less_than;
  case '>':
    goto seen_greater_than;
  case '!':
    goto seen_bang;
  case '=':
    goto seen_equals;
  case '.':
    goto seen_dot;
  case ':':
    yield(LexColon);
  case ',':
    yield(LexComma);
  case ';':
    yield(LexSemicolon);
  case '^':
    yield(LexCaret);
  case '+':
    yield(LexPlus);
  case '-':
    goto seen_minus;
  case '*':
    yield(LexAsterisk);
  case '/':
    goto seen_slash;
  default:
    lexer_error("Unexpected symbol \"%c\".", last_munched);
    return error(UNEXPECTED_SYMBOL);
  }

 seen_dot:
  switch (look_ahead(input, 0)) {
  case '.':
    munch;
    goto seen_two_dots;
  case '\n':
    munch;
    yield(LexStatementTerminator);
  default:
    yield(LexDot);
  }

 seen_two_dots:
  munch;
  switch (last_munched) {
  case '.':
    yield(LexEllipsis);
  default:
    lexer_error("Unexpected symbol \"%c\".", last_munched);
    return error(UNEXPECTED_SYMBOL);
  }

 seen_minus:
  switch (look_ahead(input, 0)) {
  case '>':
    munch;
    yield(LexRightArrow);
  default:
    yield(LexMinus);
  }

 seen_slash:
  switch (look_ahead(input, 0)) {
  case '/':
    goto skip_comment;
  default:
    yield(LexSlash);
  }

 skip_comment:
  munch;
  switch (last_munched) {
  case LINE_BREAK:
    goto start;
  case EOF:
    lexer_error("Unexpected EOF while lexing comment!\n"
		"(It is impossible to create commented lines"
		" without newlines in most text-editors, so"
		" most likely something else is seriously wrong.)");
    return error(EOF_IN_COMMENT);
  default:
    goto skip_comment;
  }

 seen_less_than:
  switch (look_ahead(input, 0)) {
  case '=':
    munch;
    yield(LexLessOrEq);
  case '-':
    munch;
    yield(LexLeftArrow);
  default:
    yield(LexLessThan);
  }
  
 seen_greater_than:
  switch (look_ahead(input, 0)) {
  case '=':
    munch;
    yield(LexGreaterOrEq);
  default:
    yield(LexGreaterThan);
  }

 seen_bang:
  munch;
  switch (last_munched) {
  case '=':
    yield(LexNotEqual);
  default:
    lexer_error("Expected \"=\" after \"!\". Found \"%c\".", last_munched);
    return error(UNEXPECTED_SYMBOL);
  }

 seen_equals:
  switch (look_ahead(input, 0)) {
  case '=':
    munch;
    yield(LexDoubleEquals);
  default:
    yield(LexEquals);
  }

  // Lexing Numbers
  // ==============
 seen_zero:
  switch (look_ahead(input, 0)) {
  case '.':
    switch(look_ahead(input, 1)) {
    case DIGIT:
      munch;
      goto scan_frac;
    default:
      yield_value(LexDecInteger);
    }
  case 'x':
  case 'X':
    munch;
    goto seen_hex;
  case 'b':
  case 'B':
    munch;
    goto seen_bin;
  default:
    yield_value(LexDecInteger);
  }

 seen_digit:
  switch (look_ahead(input, 0)) {
  case DIGIT:
    munch;
    goto seen_digit;
  case '.':
    switch(look_ahead(input, 1)) {
    case DIGIT:
      munch;
      goto scan_frac;
    default:
      yield_value(LexDecInteger);
    }
  case 'e':
  case 'E':
    munch;
    goto seen_exp;
  default:
    yield_value(LexDecInteger);
  }
  
 scan_frac:
  switch (look_ahead(input, 0)) {
  case DIGIT:
    munch;
    goto scan_frac;
  case 'e':
  case 'E':
    munch;
    goto seen_exp;
  default:
    yield_value(LexFloat);
  }
    
 seen_exp:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto scan_exp;
  case '-':
    munch;
    switch (last_munched) {
    case DIGIT:
      goto scan_exp;
    default:
      lexer_error("No digit after \"-\" in exponent! Found \"%c\".", last_munched);
      return error(UNEXPECTED_SYMBOL);
    }
  default:
    lexer_error("No exponent after \"e\". Found \"%c\".", last_munched);
    return error(UNEXPECTED_SYMBOL);
  }

 scan_exp:
  switch (look_ahead(input, 0)) {
  case DIGIT:
    munch;
    goto scan_exp;
  default:
    yield_value(LexFloat);
  }

 seen_hex:
  munch;
  switch (last_munched) {
  case HEX_DIGIT:
    goto scan_hex;
  default:
    lexer_error("No hex value after \"0x\". Found \"%c\".", last_munched);
    return error(UNEXPECTED_SYMBOL);
  }

 scan_hex:
  switch (look_ahead(input, 0)) {
  case HEX_DIGIT:
    munch;
    goto scan_hex;
  default:
    yield_value(LexHexInteger);
  }

 seen_bin:
  munch;
  switch (last_munched) {
  case '0':
  case '1':
    goto scan_bin;
  default:
    lexer_error("No binary value after \"0b\". Found \"%c\".", last_munched);
    return error(UNEXPECTED_SYMBOL);
  }

 scan_bin:
  switch (look_ahead(input, 0)) {
  case '0':
  case '1':
    munch;
    goto scan_bin;
  default:
    yield_value(LexBinInteger);
  }


  // Lexing Strings
  // ==============
 scan_string:
  munch;
  switch (last_munched) {
  case '\"':
    // Literal Quote; end of string.
    drop;
    yield_value(LexString);
  case '\\':
    // Literal Slash; escape a character.
    goto scan_escaped_character;
  case EOF:
    lexer_error("Unexpected EOF while lexing string!");
    return error(EOF_IN_STRING);
  default:
    goto scan_string;
  }

 scan_escaped_character:
  munch;
  switch (last_munched) {
  case 'n':
    drop; // drop the n
    drop; // drop the slash
    push('\n'); // push a literal newline
    goto scan_string;
  case 't':
    drop;
    drop;
    push('\t'); // push a literal tab
    goto scan_string;
  default:
    lexer_error("Unrecognized escape character \"%c\".", last_munched);
    return error(UNRECOGNISED_ESCAPE_SEQ);
  }


  // Lexing Identifiers
  // ==================
 scan_identifier:
  switch (look_ahead(input, 0)) {
  case ALPHANUMERIC:
  case '_':
  case '-': // we want these to allow functions such as str->int(string s)
  case '>':
  case '<':
    // Alphanumberic characters (a-z,A-Z,0-9) and _,-,< or > can follow
    // the first character in an identifier.
    munch;
    goto scan_identifier;
  case '?':
  case '!':
    // A question mark is legal on the end, so don't spit
    // it out! It is only legal as the last character, however,
    // so we stop searching here.
    munch;
    yield_value(id_or_keyword(munched));
  default:
    yield_value(id_or_keyword(munched));
  }
#undef lexer_error
#undef munch
#undef drop
#undef push
#undef reset_munch
#undef last_munched
#undef yield
#undef yield_eof
}

const char *lexeme_class_tostr(lexeme_class cls) {
  switch (cls) {
#define X(lclass, lrepr) case lclass: return lrepr;
    LIST_OF_LEXEMES
#undef X
  }
}

bool is_comparison(lexeme_class cls) {
  return
    cls == LexLessThan ||
    cls == LexLessOrEq ||
    cls == LexGreaterThan ||
    cls == LexGreaterOrEq ||
    cls == LexNotEqual ||
    cls == LexDoubleEquals;
}

bool is_closing_bracket(lexeme_class cls) {
  return
    cls == LexRightParenthesis ||
    cls == LexRightSquareBracket ||
    cls == LexRightCurlyBrace ||
    cls == LexGreaterThan;
}
