#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuffer.h"
#include "lexer.h"

/* Internal Stack Management
 * =========================
 * These macros manage the internal stack ("munched") in 
 * the state machine.
 */

#define munch bgetch(input, &munched[++stackp])
/* Pull a new character from the input stream,
 * and push it to the stack.
 */

#define drop stackp--
/* Drop the character on the top of the stack.
 */

#define push(element) munched[++stackp] = element
/* Push an element, not necessarily from the input
 * stream, to the stack.
 */

#define reset_munch				\
  stackp = 0;					\
  bgetch(input, &munched[stackp]);		\
  munch_start = input->column;
/* Executed when the state machine enters the
 * start-state. Prepares the stack for scanning
 * a new lexeme.
 */

#define last_munched munched[stackp]
/* Peeks the stack to get the most recently munched
 * character.
 */

/* Outputting lexemes
 * ==================
 * These macros abstract away the process of "returning" a
 * lexeme. The "yield"-nomenclature is borrowed from python,
 * since the stream preserves its state across calls to scan.
 */

#define yield(category)						\
  munched[stackp + 1] = '\0';					\
  init_lexeme(l, category, munched, input->line, munch_start);	\
  return 0

#define yield_eof							\
  init_lexeme(l, LexEndOfFile, "eof", input->line, munch_start);	\
  return 0

/* Lexer Error 
 * ===========
 */
#define lexer_error(message, ...)			       \
  fprintf(stderr,					       \
	  "Lexer Error: " message " (line %ld, column %ld)\n", \
	  ##__VA_ARGS__, input->line, input->column);	       

int init_lexeme(lexeme *l, lexeme_class cls, const char *content,
		int line, int column) {
  l->type = cls;
  l->line = line;
  l->column = column;
    
  int size_of_content = strlen(content) + 1;
  l->content = (char *) malloc(size_of_content);
  if (l->content == NULL)
    return 1;

  strcpy(l->content, content);
  return 0;
}

int free_lexeme(lexeme *l) {
  if (l == NULL)
    return FREE_NULL_LEXEME;

  free(l->content);
  l->line = 0;
  l->column = 0;
  l->content = NULL;
  return 0;
}

#define check(str, id) if (strcmp(contents, str) == 0) return id
lexeme_class id_or_keyword(const char *contents) {
  check("mod", LexMod);
  check("div", LexDiv);
  check("func", LexFunc);
  check("fn", LexFn);
  check("use", LexUse);
  check("as", LexAs);
  check("let", LexLet);
  check("if", LexIf);
  check("else", LexElse);
  check("where", LexWhere);
  check("cases", LexCases);
  check("otherwise", LexOtherwise);
  check("not", LexNot);
  check("and", LexAnd);
  check("or", LexOr);
  check("xor", LexXor);
  check("true", LexTrue);
  check("false", LexFalse);
  return LexIdentifier;
}
#undef check

int scan(ringbuffer *input, lexeme *l) {
  // Character stack
  // ===============
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

  /* -------------------------- */
  /* BEGINNING OF STATE MACHINE */
  /* -------------------------- */

 start:
  reset_munch;
  switch (last_munched) {
  case WHITESPACE:
    goto start;
  case '0':
    goto seen_zero;
  case NONZERO_DIGIT:
    goto seen_digit;
  case '\"':
    reset_munch; // get rid of the quote we just ate
    goto scan_string;
  case ALPHA: // (a-z,A-Z,_) are the only valid start of identifiers
  case '_':
    goto scan_identifier;
  case EOF:
    yield_eof;
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
    return UNEXPECTED_SYMBOL;
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
    return UNEXPECTED_SYMBOL;
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
    return EOF_IN_COMMENT;
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
    return UNEXPECTED_SYMBOL;
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
      yield(LexDecInteger);
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
    yield(LexDecInteger);
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
      yield(LexDecInteger);
    }
  case 'e':
  case 'E':
    munch;
    goto seen_exp;
  default:
    yield(LexDecInteger);
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
    yield(LexFloat);
  }
    
 seen_exp:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto scan_exp;
  case '-':
    // Verify that the character after the minus is a digit
    munch;
    switch (last_munched) {
    case DIGIT:
      goto scan_exp;
    default:
      lexer_error("No digit after \"-\" in exponent! Found \"%c\".", last_munched);
      return UNEXPECTED_SYMBOL;
    }
  default:
    lexer_error("No exponent after \"e\". Found \"%c\".", last_munched);
    return UNEXPECTED_SYMBOL;
  }

 scan_exp:
  switch (look_ahead(input, 0)) {
  case DIGIT:
    munch;
    goto scan_exp;
  default:
    yield(LexFloat);
  }

 seen_hex:
  munch;
  switch (last_munched) {
  case HEX_DIGIT:
    goto scan_hex;
  default:
    lexer_error("No hex value after \"0x\". Found \"%c\".", last_munched);
    return UNEXPECTED_SYMBOL;
  }

 scan_hex:
  switch (look_ahead(input, 0)) {
  case HEX_DIGIT:
    munch;
    goto scan_hex;
  default:
    yield(LexHexInteger);
  }

 seen_bin:
  munch;
  switch (last_munched) {
  case '0':
  case '1':
    goto scan_bin;
  default:
    lexer_error("No binary value after \"0b\". Found \"%c\".", last_munched);
    return UNEXPECTED_SYMBOL;
  }

 scan_bin:
  switch (look_ahead(input, 0)) {
  case '0':
  case '1':
    munch;
    goto scan_bin;
  default:
    yield(LexBinInteger);
  }


  // Lexing Strings
  // ==============
 scan_string:
  munch;
  switch (last_munched) {
  case '\"':
    // Literal Quote; end of string.
    drop;
    yield(LexString);
  case '\\':
    // Literal Slash; escape a character.
    goto scan_escaped_character;
  case EOF:
    lexer_error("Unexpected EOF while lexing string!");
    return EOF_IN_STRING;
  default:
    goto scan_string;
  }

 scan_escaped_character:
  munch;
  switch (last_munched) {
  case 'n':
    drop; // drop the esape code
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
    return UNRECOGNISED_ESCAPE_SEQ;
  }


  // Lexing Identifiers
  // ==================
 scan_identifier:
  switch (look_ahead(input, 0)) {
  case ALPHANUMERIC:
  case '_':
    munch;
    // Alphanumberic characters (a-z,A-Z,0-9) and _ can follow
    // the first character in an identifier.
    goto scan_identifier;
  case '?':
    munch;
    // A question mark is legal on the end, so don't spit
    // it out! It is only legal as the last character, however,
    // so we stop searching here.
    yield(id_or_keyword(munched));
  default:
    yield(id_or_keyword(munched));
  }
  
  /* -------------------- */
  /* END OF STATE MACHINE */
  /* -------------------- */
}

#define check(id) case id: return #id
const char* lexeme_class_tostr(lexeme_class cls) {
  switch (cls) {
  check(LexDecInteger);
  check(LexHexInteger);
  check(LexBinInteger);
  check(LexFloat);
  check(LexString);
  check(LexIdentifier);
  check(LexLeftParenthesis);
  check(LexRightParenthesis);
  check(LexLeftCurlyBrace);
  check(LexRightCurlyBrace);
  check(LexLeftSquareBracket);
  check(LexRightSquareBracket);
  check(LexLeftArrow);
  check(LexRightArrow);
  check(LexPlus);
  check(LexMinus);
  check(LexAsterisk);
  check(LexSlash);
  check(LexMod);
  check(LexDiv);
  check(LexEndOfFile);
  check(LexDot);
  check(LexComma);
  check(LexColon);
  check(LexSemicolon);
  check(LexEllipsis);
  check(LexLessThan);
  check(LexGreaterThan);
  check(LexLessOrEq);
  check(LexGreaterOrEq);
  check(LexEquals);
  check(LexDoubleEquals);
  check(LexNotEqual);
  check(LexFunc);
  check(LexFn);
  check(LexUse);
  check(LexAs);
  check(LexLet);
  check(LexWhere);
  check(LexIf);
  check(LexElse);
  check(LexCases);
  check(LexOtherwise);
  check(LexCaret);
  check(LexAnd);
  check(LexOr);
  check(LexXor);
  check(LexTrue);
  check(LexFalse);
  check(LexNot);
  check(LexStatementTerminator);
  check(LexNull);
  }
}
#undef check

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
