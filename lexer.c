#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"
#include "lexer.h"

/* Internal Stack Management
 * =========================
 * These macros manage the internal stack ("munched") in 
 * the state machine.
 */

#define munch munched[++stackp] = sgetc(input)
/* Pull a new character from the input stream,
 * and push it to the stack.
 */

#define spit sputc(munched[stackp--], input)
/* Pop a character from the stack, and push it
 * back into the input stream.
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
  munched[stackp] = sgetc(input);		\
  munch_start = input->column;
/* Executed when the state machine enters the
 * start-state. Prepares the stack for scanning
 * a new lexeme.
 */

#define last_munched munched[stackp]
/* Peeks the stack to get the most recently munched
 * character.
 */
  

/* Yielding Macros (returns lexemes)
 * =================================
 * Returning lexemes are abstracted away, because it is
 * relatively error-prone:
 * 
 *  - It is necessary to null-terminate the stack 
 *    that stores the characters munched since the
 *    beginning of the current lexeme, because the
 *    stack is strcpy()-ed into the returned struct.
 *
 *  - It is verbose, because a lot of information is
 *    needed to create the lexeme structure. Verbose
 *    code is error-prone because it makes you skim
 *    through it.
 * 
 * "yield"-ing is appropriate nomenclature, because the
 * stream's state persists across calls to scan(), similar
 * to iterators in languages like Python, where the notion
 * of "yielding" means the same thing.
 *
 */

#define yield(category)						\
  munched[stackp + 1] = '\0';					\
  return lexeme(category, munched, input->line, munch_start)

#define yield_eof						\
  return lexeme(EndOfFile, "eof", input->line, munch_start)

/* Lexer Error 
 * ===========
 */
#define lexer_error(message, ...)			     \
  fprintf(stderr,					     \
	  "Lexer Error: " message " (line %d, column %d)\n", \
	  ##__VA_ARGS__, input->line, input->column);		     \
  exit(1)

// This can be called through the macro lexeme() as
//   lexeme *l = lexeme(category, content, line, column);
//               ---------------------------------------
// which generalizes notation for initializing structures,
// as you don't always need a function to set it up.
lexeme *lexeme_new(lexeme_class type, const char *content, int line, int column) {
  lexeme *t = malloc(sizeof(lexeme));
  t->type = type;
  t->line = line;
  t->column = column;
    
  int conlen = strlen(content) + 1;
  t->content = (char *) malloc(conlen);
  strcpy(t->content, content);
  return t;
}

void free_lexeme(lexeme *l) {
  if (l->content != NULL)
    free(l->content);
  if (l != NULL)
    free(l);
}

#define check(str, id) if (strcmp(contents, str) == 0) return id
lexeme_class id_or_keyword(const char *contents) {
  check("mod", Mod);
  check("div", Div);
  check("func", Func);
  check("fn", Fn);
  check("use", Use);
  check("as", As);
  check("let", Let);
  check("in", In);
  check("if", If);
  check("else", Else);
  check("switch", Switch);
  check("default", Default);
  check("cases", Cases);
  check("otherwise", Otherwise);
  check("and", And);
  check("or", Or);
  check("xor", Xor);
  check("true", True);
  check("false", False);
  return Identifier;
}
#undef check

lexeme *scan(stream *input) {
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
    yield(LeftParenthesis);
  case ')':
    yield(RightParenthesis);
  case '[':
    yield(LeftSquareBracket);
  case ']':
    yield(RightSquareBracket);
  case '{':
    yield(LeftCurlyBrace);
  case '}':
    yield(RightCurlyBrace);
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
    yield(Colon);
  case ',':
    yield(Comma);
  case ';':
    yield(Semicolon);
  case '^':
    yield(Caret);
  case '+':
    yield(Plus);
  case '-':
    goto seen_minus;
  case '*':
    yield(Asterisk);
  case '/':
    goto seen_slash;
  default:
    lexer_error("Unexpected symbol \"%c\".", last_munched);
  }

 seen_dot:
  munch;
  switch (last_munched) {
  case '.':
    goto seen_two_dots;
  default:
    spit;
    yield(Dot);
  }

 seen_two_dots:
  munch;
  switch (last_munched) {
  case '.':
    yield(Ellipsis);
  default:
    lexer_error("Unexpected symbol \"%c\".", last_munched);
  }

 seen_minus:
  munch;
  switch (last_munched) {
  case NONZERO_DIGIT:
    goto seen_digit;
  case '>':
    yield(RightArrow);
  default:
    spit;
    yield(Minus);
  }

 seen_slash:
  munch;
  switch (last_munched) {
  case '/':
    goto skip_comment;
  default:
    spit;
    yield(Slash);
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
  default:
    goto skip_comment;
  }

 seen_less_than:
  munch;
  switch (last_munched) {
  case '=':
    yield(LessOrEq);
  case '-':
    yield(LeftArrow);
  default:
    spit;
    yield(LessThan);
  }
  
 seen_greater_than:
  munch;
  switch (last_munched) {
  case '=':
    yield(GreaterOrEq);
  default:
    spit;
    yield(GreaterThan);
  }

 seen_bang:
  munch;
  switch (last_munched) {
  case '=':
    yield(NotEqual);
  default:
    lexer_error("Expected \"=\" after \"!\". Found \"%c\".", last_munched);
  }

 seen_equals:
  munch;
  switch (last_munched) {
  case '=':
    yield(DoubleEquals);
  default:
    spit;
    yield(Equals);
  }

  // Lexing Numbers
  // ==============
 seen_zero:
  munch;
  switch (last_munched) {
  case '.':
    goto seen_decimal_point;
  case 'x':
  case 'X':
    goto seen_hex;
  case 'b':
  case 'B':
    goto seen_bin;
  default:
    spit;
    yield(DecInteger);
  }

 seen_digit:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto seen_digit;
  case '.':
    goto seen_decimal_point;
  case 'e':
  case 'E':
    goto seen_exp;
  default:
    spit;
    yield(DecInteger);
  }
  
 seen_decimal_point:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto scan_frac;
  default:
    spit;
    spit;
    yield(DecInteger);
  }
  
 scan_frac:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto scan_frac;
  case 'e':
  case 'E':
    goto seen_exp;
  default:
    spit;
    yield(Float);
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
    }
  default:
    lexer_error("No exponent after \"e\". Found \"%c\".", last_munched);
  }

 scan_exp:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto scan_exp;
  case '.':
    goto seen_exp_decimal_point;
  default:
    spit;
    yield(Float);
  }

 seen_exp_decimal_point:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto scan_exp_frac;
  default:
    spit;
    spit;
    yield(Float);
  }

 scan_exp_frac:
  munch;
  switch (last_munched) {
  case DIGIT:
    goto scan_exp_frac;
  default:
    spit;
    yield(Float);
  }

 seen_hex:
  munch;
  switch (last_munched) {
  case HEX_DIGIT:
    goto scan_hex;
  default:
    lexer_error("No hex value after \"0x\". Found \"%c\".", last_munched);
  }

 scan_hex:
  munch;
  switch (last_munched) {
  case HEX_DIGIT:
    goto scan_hex;
  default:
    spit;
    yield(HexInteger);
  }

 seen_bin:
  munch;
  switch (last_munched) {
  case '0':
  case '1':
    goto scan_bin;
  default:
    lexer_error("No binary value after \"0b\". Found \"%c\".", last_munched);
  }

 scan_bin:
  munch;
  switch (last_munched) {
  case '0':
  case '1':
    goto scan_bin;
  default:
    spit;
    yield(BinInteger);
  }


  // Lexing Strings
  // ==============
 scan_string:
  munch;
  switch (last_munched) {
  case '\"':
    // Literal Quote; end of string.
    drop;
    yield(String);
  case '\\':
    // Literal Slash; escape a character.
    goto scan_escaped_character;
  case EOF:
    lexer_error("Unexpected EOF while lexing string!");
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
  }


  // Lexing Identifiers
  // ==================
 scan_identifier:
  munch;
  switch (last_munched) {
  case ALPHANUMERIC:
  case '_':
    // Alphanumberic characters (a-z,A-Z,0-9) and _ can follow
    // the first character in an identifier.
    goto scan_identifier;
  case '?':
    // A question mark is legal on the end, so don't spit
    // it out! It is only legal as the last character, however,
    // so we stop searching here.
    yield(id_or_keyword(munched));
  default:
    spit;
    yield(id_or_keyword(munched));
  }
  
  /* -------------------- */
  /* END OF STATE MACHINE */
  /* -------------------- */
}

#define check(id) case id: return #id
const char* lexeme_class_tostr(lexeme_class c) {
  switch (c) {
  check(DecInteger);
  check(HexInteger);
  check(BinInteger);
  check(Float);
  check(String);
  check(Identifier);
  check(LeftParenthesis);
  check(RightParenthesis);
  check(LeftCurlyBrace);
  check(RightCurlyBrace);
  check(LeftSquareBracket);
  check(RightSquareBracket);
  check(LeftArrow);
  check(RightArrow);
  check(Plus);
  check(Minus);
  check(Asterisk);
  check(Slash);
  check(Mod);
  check(Div);
  check(EndOfFile);
  check(Dot);
  check(Comma);
  check(Colon);
  check(Semicolon);
  check(Ellipsis);
  check(LessThan);
  check(GreaterThan);
  check(LessOrEq);
  check(GreaterOrEq);
  check(Equals);
  check(DoubleEquals);
  check(NotEqual);
  check(Func);
  check(Fn);
  check(Use);
  check(As);
  check(Let);
  check(In);
  check(If);
  check(Else);
  check(Switch);
  check(Default);
  check(Cases);
  check(Otherwise);
  check(Caret);
  check(And);
  check(Or);
  check(Xor);
  check(True);
  check(False);
  default:
    return "<tostr() not implemented for this lexeme class>";
  }
}
#undef check
