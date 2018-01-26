#include "../lexer.h"
#include "../stream.h"
#include <stdio.h>

#include "test.h"

suite(stream, { 
    // Verify that the source is not pointing to null.
    test("create stream from file", {
	stream *s = stream_fromfile("ptest");
	assert(s->source != NULL);
	sclose(s);
    });
    
    // Verify that the source IS pointing to null. 
    test("create stream from string", {
	stream *s = stream_fromstr("");
	assert(s->source == NULL);
	sclose(s);
    });
    
    test("sgetc (from string)", {
	stream *s = stream_fromstr("abc");
	assert(sgetc(s) == 'a');
	assert(sgetc(s) == 'b');
	assert(sgetc(s) == 'c');
	sclose(s);
    });

    // if this suddenly broke, make sure the
    // file is still the same :-)
    test("sgetc (from file)", {
	stream *s = stream_fromfile("ptest");
	assert(sgetc(s) == '0');
	assert(sgetc(s) == ' ');
	assert(sgetc(s) == '1');
	sclose(s);
    });
    
    test("empty stream yields EOF forever", {
	stream *s = stream_fromstr("");
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	sclose(s);
    });
    
    test("push and read from internal stack", {
	stream *s = stream_fromstr("");
	assert(sgetc(s) == EOF);
	sputc('e', s);
	sputc('i', s);
	sputc('l', s);
	assert(s->stackp == 3);
	assert(sgetc(s) == 'l');
	assert(sgetc(s) == 'i');
	assert(sgetc(s) == 'e');
	assert(s->stackp == 0);
    });
    
    // this test possibly tests too many things...
    test("columns and lines (\"cursor\")", {
	stream *s = stream_fromstr("foo\nbar\n");
	
	// read to end of first line.
	while(sgetc(s) != '\n')
	  ;
	assert(s->line == 1);
	assert(s->column == 4);

	// read one more, we expect a newline
	// and a "carrige return".
	sgetc(s);
	assert(s->line == 2);
	assert(s->column == 1);
	
	// pushing characters back does not
	// moodify the cursor position.
	sputc('l', s);
	sputc('o', s);
	sputc('o', s);
	assert(s->line == 2);
	assert(s->column == 1);
	
	// read to end of second line
	while(sgetc(s) != '\n')
	  ;
	
	// last character should be EOF,
	assert(sgetc(s) == EOF);
    });

})

int main() {
  run_suite(stream);
  return 0;
}
