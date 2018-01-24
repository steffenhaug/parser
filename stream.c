#include <stdio.h>
#include <stdlib.h>
#include "stream.h"
#include <string.h>

stream *stream_fromfile(const char *filename) {
  stream *t = malloc(sizeof(stream));

  // open file
  t->source = fopen(filename, "r");
  if (t->source == NULL) {
    fprintf(stderr, "stream.c ERROR: file does not exist! (%s)\n", filename);
    exit(1);
  }
  t->fname = filename;

  // prefill buffer, and zero buffer position
  t->nread = fread(t->buffer, 1, BUFFER_SIZE, t->source);
  t->bufferp = 0;

  // line/column "cursor"
  t->line = 1;
  t->column = 0;
  t->break_line = 0;
  return t;
}

stream *stream_fromstr(const char* str) {
  stream *t = malloc(sizeof(stream));

  // point source to NULL
  t->source = NULL;
  t->fname = "<internal memory buffer>";
  // place string in buffer, and zero buffer position
  strcpy(t->buffer, str);
  t->bufferp = 0;

  // one additional character for EOF
  t->nread = strlen(str) + 1;
  
  // place EOF at the end of string, so the stream terminates
  t->buffer[strlen(str)] = EOF;
  
  // line/column "cursor"
  t->line = 1;
  t->column = 0;
  t->break_line = 0;
  return t;
}

char sgetc(stream *s) {
  char r = EOF;
  if (s->bufferp >= BUFFER_SIZE && s->source != NULL) {
    // we have read the buffer to the end,
    // and s is a file stream (source is not NULL).
    // buffer the next part of the file:
    s->nread = fread(s->buffer, 1, BUFFER_SIZE, s->source);
    // and reset the buffer-position.
    s->bufferp = 0;
  }
  if (s->stackp > 0)
    // there are characters on the stack.
    return s->stack[--(s->stackp)];
  if (s->bufferp < s->nread)
    // there ARE characters left in the buffer.
    r =  s->buffer[(s->bufferp)++];
  else if (s->source != NULL) {
    // there ARE NO characters left in the buffer.
    fclose(s->source);
  }

  // line/column "cursor" management
  if (s->break_line) {
    // the previous character was '\n'
    s->column = 0;
    s->line++;
  }

  // set break_line flag
  if (r == '\n')
    s->break_line = 1;
  else
    s->break_line = 0;

  s->column++;
  return r;
}

void sputc(char c, stream *s) {
  if (s->stackp > STACK_SIZE) {
    fprintf(stderr, "stream.c ERROR: sputc-stack is full! (%s)\n", s->fname);
    exit(1);
  }
  else
    s->stack[s->stackp++] = c;
}

void sclose(stream *s) {
  if (s->source != NULL)
    fclose(s->source);
  free(s);
}
