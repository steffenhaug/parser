#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/* ringbuffer
 * ==========
 * Implements a ringbuffer to read files with lookahead
 * You can advance the file character by character, like
 * a stream, and the file will be buffered in batches
 * under the hood.
 * 
 * You can also create ringbuffers from strings, providing
 * an abstraction for consuming sequences of characters.
 */

// Smaller batches with -DLIE_DEBUG, so it is easy
// to test the wrap-around of the buffer without reading
// a million characters from a big file
#ifdef LIE_DEBUG
#define BATCH_SIZE 4
#else
#define BATCH_SIZE 128
#endif // LIE_DEBUG

#define BUFFER_SIZE (3 * BATCH_SIZE)

/* Error Codes */
#define STRING_LONGER_THAN_BUFFER 201
#define COULD_NOT_OPEN_FILE 202
#define ADVANCE_FREE_BUFFER 203
#define SOURCE_EXHAUSTED 204
#define INVALID_BUFFER_TYPE 205

typedef enum {
  StringBuffer,
  FileBuffer,
} buffer_type;

typedef struct {
  FILE *fileptr;
  char buffer[BUFFER_SIZE];
} buffered_file;

typedef struct {
  // Type and name for housekeeping
  buffer_type type;
  const char *name;
  // Whether we have read the entire source or not
  bool exhausted;

  union {
    buffered_file as_file;
    const char *as_str;
  } source;
  
  // character "underneath the cursor",
  // i.e. the previous characrer read
  char at_cursor;

  // The position in the buffer, and the last valid position
  size_t position, buffer_limit;
  
  // The "cursor" (position in the source)
  size_t line, column;
} ringbuffer;

int init_filebuffer(ringbuffer *b, const char* filename);
// Create a ringbuffer from a file.

int init_stringbuffer(ringbuffer *b, const char *str);
// Create a ringbuffer from a string

int free_ringbuffer(ringbuffer *b);
// Closes the underlying file if the buffer is
// a FileBuffer. Sets everything to NULL or zero.

int get_character(ringbuffer *b, char *c);
// put the next character in c

char look_ahead(ringbuffer *b, size_t i);
// return the character i positions after the current
// look_ahead(&b 0) yields the character that comes from
// bgetch next time it is called.
