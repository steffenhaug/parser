/* stream.h
   --------
   Implements a stream-esque data structure to deal with
   file I/O in a (slightly) more abstract manner. This is
   achieved by abstracting away the buffer-management, so
   the user don't need to manually refill the buffer with
   new data when the current is read to the end.
   
   The structure also has a stack that the user can push
   characters to, in which case these will be returned
   first, before any characters are read from the buffer.
*/

#define BUFFER_SIZE 512
#define STACK_SIZE 32

struct stream {
  FILE *source;
  const char *fname;
  char buffer[BUFFER_SIZE];
  char stack[STACK_SIZE];
  size_t nread;
  int bufferp;
  int stackp;
};

typedef struct stream stream;

stream *sopen(const char *filename);
/* Opens a stream. ([s]tream[open])
   Allocates memory for a stream, and opens the internal file.
 */

void sclose(stream *s);
/* Closes the stream. ([s]tream[close])
   The internal file is closed, even if it has not been read
   to the end. The structure is deallocated.
 */

char sgetc(stream *s);
/* Get the next character from a stream. ([s]tream[get][c]har)

   If there are characters in the stack, those are returned
   first, otherwise characters are read from the buffer. If
   the buffer is read to the end, the buffer is refilled with
   the next piece of the file automatically.

   When the end of the file is reached, the internal file is
   closed, but the structure is not deallocated. You can in
   other words still push characters back on the stack, and
   it will work. If the stack is empty after the file is
   closed, we simply return EOF for ever.
*/


void sputc(char c, stream *s);
/* Push a character back in the stream. ([s]tream[put][c]har)
   
   The char is pushed onto the streams internal stack, and as
   such will be read on the next call to sgetc.

   If you have to use an algorithm that relies heavily on
   backtracking, you may want to increase the stack size, as
   the stack is pretty shallow by default.
 */
