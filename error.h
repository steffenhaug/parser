/*
 * Error Codes
 * ===========
 * The convention is the following:
 *  - ok (0) is success,
 *  - Negative numbers are errors i.e. something went wrong,
 *  - Positive numbers are warnings, i.e. a special case of an 
 *    algorithm, or something similar, but nothing went "wrong"
 * The macros are preferred to because they signal intent.
 */

#include <stdbool.h>

#define error(code) (-(code))
#define warning(code) (code)

#define error_q(code) ((code) < 0 ? true : false)
#define warning_q(code) ((code) > 0 ? true : false)

// Generic
#define ok 0
#define fail 1

// Memory 31-100
#define FREE_NULL 31
#define FAILED_MALLOC 32

// Ringbuffer, 201-299
#define STRING_LONGER_THAN_BUFFER 201
#define COULD_NOT_OPEN_FILE 202
#define ADVANCE_FREE_BUFFER 203
#define SOURCE_EXHAUSTED 204
#define INVALID_BUFFER_TYPE 205

// Lexer, 301-399
#define FREE_NULL_LEXEME 301
#define UNEXPECTED_SYMBOL 302
#define EOF_IN_COMMENT 303
#define EOF_IN_STRING 304
#define UNRECOGNISED_ESCAPE_SEQ 305
#define FAILED_MALLOC_CONTENT 306

// Parser, 401-499
#define MATCH_FAILED 401
#define MATCH_STORE_NO_VALUE 402
#define EXPECTED_ATOM 403
