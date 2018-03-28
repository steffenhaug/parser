#include <stdio.h>

#include "parser.h"

/*
 * Parser Management
 * =================
 * advance, match, LA and LT are used to encode the
 * grammar in a recursive descent parser.
 */

int advance(parser *p) {
  // Free the previous lexeme
  free_lexeme(p->lookahead[p->position]);

  // Get a new one in its place
  p->lookahead[p->position] = scan(p->input);

  // Advance the position, potentially wrapping around
  p->position = (p->position + 1) % MAX_LOOKAHEAD;
  return 0;
}

int init_parser(parser *p, stream *s) {
  if (s == NULL) {
    parser_error("Failed to initialize Parser! Stream is NULL.");
    return STREAM_IS_NULL;
  }

  p->input = s;
  p->position = 0;
  
  // Fill the lookahead ring-buffer
  for (int i = 0; i < MAX_LOOKAHEAD; i++) {
    p->lookahead[i] = scan(p->input);
  }

  return 0;
}

lexeme *LA(parser *p, size_t i) {
  return p->lookahead[i + p->position];
}

lexeme_class LT(parser *p, size_t i) {
  return LA(p, i)->type;
}

int match(parser *p, lexeme_class cls) {
  // Assert that the current lexeme has the right type
  if (LT(p, 0) != cls) {
    parser_error("Failed to match %s, found %s. (line: %d, column: %d)",
		 lexeme_class_tostr(cls),
		 lexeme_class_tostr(LT(p, 0)),
		 LA(p, 0)->line,
		 LA(p, 0)->column);
    return MATCH_FAILED;
  }

  advance(p);
  return 0;
}
