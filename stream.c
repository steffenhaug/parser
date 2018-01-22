#include <stdio.h>
#include <stdlib.h>
#include "stream.h"

stream *sopen(const char *filename) {
  stream *t = malloc(sizeof(stream));
  t->source = fopen(filename, "r");
  if (t->source == NULL) {
    fprintf(stderr, "stream.c ERROR: file does not exist! (%s)\n", filename);
    exit(1);
  }
  t->fname = filename;
  t->nread = fread(t->buffer, 1, BUFFER_SIZE, t->source);
  return t;
}

char sgetc(stream *s) {
  if (s->bufferp >= BUFFER_SIZE) {
    // we have read the buffer to the end.
    // buffer the next part of the file:
    s->nread = fread(s->buffer, 1, BUFFER_SIZE, s->source);
    // and reset the buffer-position.
    s->bufferp = 0;
  }
  if (s->stackp > 0)
    // there are characters on the stack.
    return s->stack[--(s->stackp)];
  else if (s->bufferp < s->nread)
    // there are characters left in the buffer.
    return s->buffer[(s->bufferp)++];
  else {
    // stack is empty, and there is nothing more to buffer.
    fclose(s->source);
    return EOF;
  }
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
  fclose(s->source);
  free(s);
}
