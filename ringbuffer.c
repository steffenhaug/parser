#include <string.h>

#include "ringbuffer.h"

#define ringbuffer_error(message, ...)			     \
  fprintf(stderr,					     \
	  "Ringbuffer Error: " message "\n", ##__VA_ARGS__);

#define source_fileptr(buf) ((buf)->source.as_file.fileptr)
#define source_buffer(buf) ((buf)->source.as_file.buffer)

#define source_string(buf) ((buf)->source.as_str)

// imlplements the modulus operator, which wraps
// around correctly even when m is negative.
int mod(int m, int n) {
  return (m % n + n) % n;
}

void exhaust(ringbuffer *b) {
    b->exhausted = true;
}

bool is_exhausted(ringbuffer *b) {
  return b->exhausted;
}

size_t read_limit(ringbuffer *b) {
  // the limit for how far we can read before next batch
  switch (b->type) {
  case FileBuffer:
    return mod(b->buffer_limit - BATCH_SIZE, BUFFER_SIZE);
  case StringBuffer:
    return b->buffer_limit;
  default:
    ringbuffer_error("Invalid buffer type! You probably forgot to initialize it. (type: %d)", b->type);
    return 0;
  }
}

int init_filebuffer(ringbuffer *b, const char *filename) {
  b->type = FileBuffer;
  b->name = filename;
  
  source_fileptr(b) = fopen(filename, "r");
  if (source_fileptr(b) == NULL)
    goto failed_open;

  b->position = 0;

  // read two batches
  int nread = fread(source_buffer(b), sizeof(char), 2 * BATCH_SIZE,
		    source_fileptr(b));

  // instantly exhaust buffer if it is empty
  if (nread) {
    b->buffer_limit = nread - 1;
    b->exhausted = false;
  } else {
    b->buffer_limit = 0;
    exhaust(b);
  }

  b->at_cursor = (char) NULL;

  // Line  starts  at one,  column  at  zero,  because the  column  is
  // incremented immediately, as we "advance" to the first character!
  b->line = 1;
  b->column = 0;

  return ok;

 failed_open:
  ringbuffer_error("Failed to open file! (%s)", filename);
  free_ringbuffer(b);
  return error(COULD_NOT_OPEN_FILE);
}

int init_stringbuffer(ringbuffer *b, const char *str) {
  b->type = StringBuffer;
  b->name = "<string buffer>";

  source_string(b) = str;
  b->at_cursor = (char) NULL;
  
  b->position = 0;

  size_t length = strlen(str);
  if (length) {
    b->buffer_limit = length - 1;
    b->exhausted = false;
  } else {
    b->buffer_limit = 0;
    exhaust(b);
  }

  b->line = 1;
  b->column = 0;
  return ok;
}

int free_ringbuffer(ringbuffer *b) {
  if (b->type == FileBuffer) {
    fclose(source_fileptr(b));
    b->source.as_file.fileptr = NULL;
  }

  memset(source_buffer(b), 0, BUFFER_SIZE);
  b->at_cursor = 0;

  b->position = 0;
  b->buffer_limit = 0;
  b->line = 0;
  b->column = 0;
  return ok;
}


int advance_filebuffer(ringbuffer *b, char *c) {
  *c = source_buffer(b)[b->position];
  b->at_cursor = *c;

  if (b->position == b->buffer_limit)
    exhaust(b);

  if (b->position == read_limit(b)) {
    // get a pointer into the buffer
    size_t next_index = mod(b->buffer_limit + 1, BUFFER_SIZE);
    char *next_start = &source_buffer(b)[next_index];

    int nread = fread(next_start, sizeof(char), BATCH_SIZE, source_fileptr(b));

    b->buffer_limit = mod(b->buffer_limit + nread, BUFFER_SIZE);
  }

  b->position = mod(b->position + 1, BUFFER_SIZE);
  return ok;
}

int advance_stringbuffer(ringbuffer *b, char *c) {
  *c = source_string(b)[b->position];
  b->at_cursor = *c;

  if (b->position == b->buffer_limit)
    exhaust(b);

  b->position++;
  return ok;
}

int get_character(ringbuffer *b, char *c) {
  if (b->at_cursor == '\n') {
    b->line++;
    b->column = 1;
  } else if (b->at_cursor != EOF) {
    b->column++;
  }
  
  if (is_exhausted(b))
    goto exhausted;

  switch (b->type) {
  case FileBuffer:
    return advance_filebuffer(b, c);
  case StringBuffer:
    return advance_stringbuffer(b, c);
  default:
    goto invalid_buffer_type;
  }

 exhausted:
  *c = EOF;
  b->at_cursor = EOF;
  return warning(SOURCE_EXHAUSTED);

 invalid_buffer_type:
  *c = (char) NULL;
  b->at_cursor = (char) NULL;
  ringbuffer_error("Invalid buffer type! You probably forgot to initialize it. (type: %d)", b->type);
  return error(INVALID_BUFFER_TYPE);
}

char look_ahead(ringbuffer *b, size_t i) {
  if (i > BATCH_SIZE)
    goto looked_too_far;

  size_t target = b->position + i;
  if (target > b->buffer_limit)
    goto looked_past_end;

  switch (b->type) {
  case FileBuffer:
    return source_buffer(b)[target];
  case StringBuffer:
    return source_string(b)[target];
  default:
    goto invalid_buffer_type;
  }

 invalid_buffer_type:
  ringbuffer_error("Invalid buffer type! You probably forgot to initialize it. (type: %d)", b->type);
  return (char) NULL;

 looked_too_far:
  ringbuffer_error("Tried to look outside buffer! (buffer name: %s)", b->name);
  return EOF;

 looked_past_end:
  return EOF;
}
