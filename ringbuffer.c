#include <string.h>

#include "ringbuffer.h"

int init_filebuffer(ringbuffer *b, const char *filename) {
  b->type = FileBuffer;

  b->filename = filename;
  b->source = fopen(filename, "r");
  if (b->source == NULL) {
    ringbuffer_error("Failed to open file! (%s)", filename);
    return COULD_NOT_OPEN_FILE;
  }

  b->position = 0;
  b->last_position = fread(b->buffer, sizeof(char), BATCH_SIZE, b->source) - 1;

  // Line starts at one, column at zero, because
  // column is incremented immediately as we "advance"
  // to the first character!
  b->line = 1;
  b->column = 0;
  return 0;
}


int init_stringbuffer(ringbuffer *b, const char *str) {
  size_t length = strlen(str);

  if (length + 1 > BUFFER_SIZE)
    return STRING_LONGER_THAN_BUFFER;

  b->type = StringBuffer;

  b->filename = "<string buffer>";
  b->source = NULL;

  strcpy(b->buffer, str);
  b->buffer[length] = EOF;
  b->previous = 0;

  b->position = 0;
  b->last_position = length;

  // Line starts at one, column at zero, because
  // column is incremented immediately as we "advance"
  // to the first character!
  b->line = 1;
  b->column = 0;
  return 0;
}

int free_ringbuffer(ringbuffer *b) {
  b->type = FreeBuffer;
  b->filename = "<free buffer>";
  if (b->source != NULL) {
    fclose(b->source);
    b->source = NULL;
  }

  for (int i = 0; i < BUFFER_SIZE; i++)
    b->buffer[i] = 0;

  b->previous = 0;

  b->position = 0;
  b->last_position = 0;

  b->line = 0;
  b->column = 0;
  return 0;
}


int advance_filebuffer(ringbuffer *b, char *c) {
  if (b->source == NULL) {
    *c = EOF;
  } else {
    *c = b->buffer[b->position];
  }
  b->previous = *c;

  // if we are at the end of the loaded range
  if (b->position == b->last_position) {
    // the start  of the next  batch is  the character after  the last
    // currently valid, potentially wrapped around!
    size_t next_index = (b->last_position + 1) % BUFFER_SIZE;
    char *next_start = &b->buffer[next_index];

    // read one batch from the source, into the buffer, starting at
    // the position 'next_start'.
    int nread = fread(next_start, sizeof(char), BATCH_SIZE, b->source);

    // move the position of the last element
    b->last_position = (b->last_position + nread) % BUFFER_SIZE;

    // if we can't read more (nread == 0), we are done.
    // we close the file, and upon further calls we detect
    // this, and return EOF forever.
    if (nread == 0) {
      fclose(b->source);
      b->source = NULL;
    }
  }

  b->position = (b->position + 1) % BUFFER_SIZE;

  return 0;
}

int advance_stringbuffer(ringbuffer *b, char *c) {
  if (b->position > b->last_position) {
    *c = EOF;
  } else {
    *c = b->buffer[b->position];
  }
  b->previous = *c;
  b->position++;
  return 0;
}

int bgetch(ringbuffer *b, char *c) {
  int error_code;

  // yield EOF forever after the end of the stream
  if (b->previous == EOF) {
    *c = EOF;
    return 0;
  }

  // if the previous character was a line break, this
  // character is on the beginning of a new line
  if (b->previous == '\n') {
    b->line++;
    b->column = 1;
  } else {
    b->column++;
  }

  switch (b->type) {
  case FileBuffer:
    error_code = advance_filebuffer(b, c);
    break;
  case StringBuffer:
    error_code = advance_stringbuffer(b, c);
    break;
  case FreeBuffer:
    ringbuffer_error("Attempt to advance a freed buffer!");
    error_code = ADVANCE_FREE_BUFFER;
  }

  return error_code;
}

char look_ahead(ringbuffer *b, size_t i) {
  size_t target = b->position + i;
  if (target > b->last_position) {
    if (b->source != NULL)
      ringbuffer_error("Tried to look outside buffer!");
    return EOF;
  } else {
    return b->buffer[target];
  }
}
