#pragma once

#include <stdio.h>
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

#define BATCH_SIZE 256
#define BUFFER_SIZE (3 * BATCH_SIZE)

/* Error Codes */
#define STRING_LONGER_THAN_BUFFER 201
#define COULD_NOT_OPEN_FILE 202
#define ADVANCE_FREE_BUFFER 203

#define ringbuffer_error(message, ...)			     \
  fprintf(stderr,					     \
	  "Ringbuffer Error: " message "\n", ##__VA_ARGS__);


typedef enum {
  StringBuffer,
  FileBuffer,
  FreeBuffer,
} buffer_type;

typedef struct {
  buffer_type type;
  // If the type is FileBuffer we keep track of the source
  const char *filename;
  FILE *source;
  // Buffer keeps the next few cahracters 
  char buffer[BUFFER_SIZE];
  // We need the previous character for line breaks
  char previous;
  // The position in the buffer (!)
  size_t position;
  // The position of the last (valid) character in the buffer
  size_t last_position;
  // The position (in the file or string) of 
  // the previous (!) character
  size_t line;
  size_t column;
} ringbuffer;

int init_filebuffer(ringbuffer *b, const char* filename);
// Create a ringbuffer from a file.

int init_stringbuffer(ringbuffer *b, const char *str);
// Create a ringbuffer from a string

int free_ringbuffer(ringbuffer *b);
// Closes the underlying file if the buffer is
// a FileBuffer. Sets everything to NULL or zero.

int bgetch(ringbuffer *b, char *c);
// put the next character in c

char look_ahead(ringbuffer *b, size_t i);
// return the character i positions after the current
// look_ahead(&b 0) yields the character that comes from
// bgetch next time it is called.
