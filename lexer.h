/* Lexer.h
 * =======
 * The lexer is,  in principle,  capable of arbitrary
 * lookahead, if you replace that stack - which is of
 * fixed size - with a dynamically allocated array.
 *   This is  assuming  it is  possible to  push back
 * arbitrarily  many characters  to the input source,
 * which can easily be  achieved with another dynamic
 * array.
 *   Lexing Lie does not require arbitrary lookahead,
 * which is why  I  opted for a rather shallow stack.
 * 
 * Much of  the heavy lifting  is  accomplished using
 * macros, for practical reasons.  The internal state
 * machine also  uses  goto,  because  state machines
 * based  on  arrays of function pointers,  and other
 * classic tricks are (very) cumbersome.
 *   The  resulting  state machine  is  very straight 
 * forward,  as long as you know how the macros work,
 * and they are quite simple.
 */

#pragma once
#include "stream.h"

// THe maximum stack depth is  also the max length of
// strings, identifiers and so on.
#define LEXEME_STACK_DEPTH 512

/* Equivalence Classes
 * -------------------
 * These are _ONLY_ supposed to be used within the state
 * machine. They look very weird, but remember that they
 * expand in the context
 * 
 *   case MACRO:
 *     // do stuff;
 * 
 * and that is why the first case has no "case", and the
 * last case lacks a colon.
 *   This method, of generating large walls of case-clauses,
 * would look super intimidating if it not automated --
 * which is why it is automated. It has the advantage over
 * other approaches of fitting nicely into other switch-
 * statements, and causing minimal work at runtime, since
 * the compiler just generates a jump table.
 */
#define DIGIT					\
  '0':						\
  case '1':					\
  case '2':					\
  case '3':					\
  case '4':					\
  case '5':					\
  case '6':					\
  case '7':					\
  case '8':					\
  case '9'

#define NONZERO_DIGIT				\
  '1':						\
  case '2':					\
  case '3':					\
  case '4':					\
  case '5':					\
  case '6':					\
  case '7':					\
  case '8':					\
  case '9'

#define HEX_DIGIT				\
  '0':						\
  case '1':					\
  case '2':					\
  case '3':					\
  case '4':					\
  case '5':					\
  case '6':					\
  case '7':					\
  case '8':					\
  case '9':					\
  case 'a':					\
  case 'b':					\
  case 'c':					\
  case 'd':					\
  case 'e':					\
  case 'f':					\
  case 'A':					\
  case 'B':					\
  case 'C':					\
  case 'D':					\
  case 'E':					\
  case 'F'

#define LOWERCASE_ALPHA				\
  'a':						\
  case 'b':					\
  case 'c':					\
  case 'd':					\
  case 'e':					\
  case 'f':					\
  case 'g':					\
  case 'h':					\
  case 'i':					\
  case 'j':					\
  case 'k':					\
  case 'l':					\
  case 'm':					\
  case 'n':					\
  case 'o':					\
  case 'p':					\
  case 'q':					\
  case 'r':					\
  case 's':					\
  case 't':					\
  case 'u':					\
  case 'v':					\
  case 'w':					\
  case 'x':					\
  case 'y':					\
  case 'z'					\

#define UPPERCASE_ALPHA				\
  'A':						\
  case 'B':					\
  case 'C':					\
  case 'D':					\
  case 'E':					\
  case 'F':					\
  case 'G':					\
  case 'H':					\
  case 'I':					\
  case 'J':					\
  case 'K':					\
  case 'L':					\
  case 'M':					\
  case 'N':					\
  case 'O':					\
  case 'P':					\
  case 'Q':					\
  case 'R':					\
  case 'S':					\
  case 'T':					\
  case 'U':					\
  case 'V':					\
  case 'W':					\
  case 'X':					\
  case 'Y':					\
  case 'Z'					\

#define ALPHA					\
  LOWERCASE_ALPHA:				\
  case UPPERCASE_ALPHA

#define ALPHANUMERIC				\
  LOWERCASE_ALPHA:				\
  case UPPERCASE_ALPHA:				\
  case DIGIT

#define WHITESPACE				\
  ' ':						\
  case '\t':					\
  case '\n':					\
  case '\r'

#define WHITESPACE_NO_LINE_BREAK		\
  ' ':						\
  case '\t'

#define LINE_BREAK				\
  '\n':						\
  case '\r'

/* Lexeme Structure
 * ================
 * Each lexeme has a 
 *   type: (lexeme_class)
 *     To identify the Lexeme as a member of its class
 *   contents: (char*)
 *     Keeps additional information, for example the
 *     value of numbers.
 *   line, column: (int)
 *     The position the elxeme was found in the input
 *     stream. This helps with reporting errors.
 */

// ... X macro?
typedef enum {
  // "Container" Lexemes
  DecInteger, // Decimal integers, like 1024
  HexInteger, // Hexadecimal integers, like 0xFF00FF
  BinInteger, // Binary integers, like 0b11011
  Float,      // Floats, like 12.3, but also 12e3 (!) and 1.2e3 and 1e2.3 
  String,     // Strings, like "hello, world"
  Identifier, // Names identifiyng things like variables
  // Brackets
  LeftParenthesis,    // (
  RightParenthesis,   // )
  LeftCurlyBrace,     // {
  RightCurlyBrace,    // }
  LeftSquareBracket,  // [
  RightSquareBracket, // ]
  // Operators (< and > can also be brackets, of course)
  LessThan,     // <
  GreaterThan,  // >
  LessOrEq,     // <=
  GreaterOrEq,  // >=
  Equals,       // =
  DoubleEquals, // ==
  NotEqual,     // !=
  LeftArrow,    // <-
  RightArrow,   // ->
  Dot,          // .
  Comma,        // ,
  Colon,        // :
  Semicolon,    // ;
  Ellipsis,     // ...
  Minus,        // -
  Plus,         // +
  Asterisk,     // *
  Slash,        // /
  Caret,        // ^
  // Reserved Keywords
  Div, Mod,
  And, Or, Xor,
  True, False,
  Func,
  Fn,
  Use,
  As,
  Let,
  In,
  If,
  Else,
  Switch,
  Default,
  Cases,
  Otherwise,
  // EOF
  EndOfFile,
} lexeme_class;

typedef struct {
  lexeme_class type;
  char *content;
  int line;
  int column;
} lexeme;

#define lexeme(type, content, line, column)	\
  lexeme_new(type, content, line, column)

lexeme *lexeme_new(lexeme_class type, const char* content, int l, int c);

void free_lexeme(lexeme *l);

const char* lexeme_class_tostr(lexeme_class c);

lexeme *scan(stream *s);
