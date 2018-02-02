#include <stdio.h>
#include <string.h>

#include "test.h"
#include "../lexer.h"
#include "../stream.h"

suite(stream, { 
    // Verify that the source is not pointing to null.
    unit("create stream from file", {
	stream *s = stream_fromfile("ptest");
	assert(s->source != NULL);
	sclose(s);
    });
    
    // Verify that the source IS pointing to null. 
    unit("create stream from string", {
	stream *s = stream_fromstr("");
	assert(s->source == NULL);
	sclose(s);
    });
    
    unit("sgetc (from string)", {
	stream *s = stream_fromstr("abc");
	assert(sgetc(s) == 'a');
	assert(sgetc(s) == 'b');
	assert(sgetc(s) == 'c');
	sclose(s);
    });

    // if this suddenly broke, make sure the
    // file is still the same :-)
    unit("sgetc (from file)", {
	stream *s = stream_fromfile("ptest");
	assert(sgetc(s) == '0');
	assert(sgetc(s) == ' ');
	assert(sgetc(s) == '1');
	sclose(s);
    });
    
    unit("empty stream yields EOF forever", {
	stream *s = stream_fromstr("");
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	assert(sgetc(s) == EOF);
	sclose(s);
    });
    
    unit("push and read from internal stack", {
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
    
    // this unit possibly tests too many things...
    unit("columns and lines (\"cursor\")", {
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

	assert(s->line == 2);
	assert(s->column == 4);
	
	// last character should be EOF,
	assert(sgetc(s) == EOF);
    });

    unit("column and line does not move past EOF", {
	stream *s = stream_fromstr("e");
	sgetc(s);
	assert(s->line == 1);
	assert(s->column == 1);

	// now we get EOF
	sgetc(s);
	sgetc(s);
	sgetc(s);
	sgetc(s);
	assert(s->line == 1);
	assert(s->column == 2);
	sclose(s);
    });

})

suite(lexer, {
    unit("create and delete lexeme", {
	lexeme *l = lexeme_new(END_OF_FILE, "eof", 1, 1);
	assert(l != NULL);
	
	// check that no members are left uninitialized
	assert(l->category == END_OF_FILE);
	assert(strcmp(l->content, "eof") == 0);
	assert(l->line == 1);
	assert(l->column == 1);

	// delete it
	lexeme_delete(l);
    });

    unit("column and line of the lexeme", {
	stream *s = stream_fromstr("1 3 50 80");
	lexeme *l;

	l = scan(s);
	assert(l->line == 1);
	assert(l->column == 1);

	l = scan(s);
	assert(l->line == 1);
	assert(l->column == 3);

	l = scan(s);
	assert(l->line == 1);
	assert(l->column == 5);
	
	l = scan(s);
	assert(l->line == 1);
	assert(l->column == 8);

	// EOF lexeme have "its own" column
	l = scan(s);
	assert(l->line == 1);
	assert(l->column == 10);
	assert(l->category == END_OF_FILE);

	// try to advance past EOF
	l = scan(s);
	l = scan(s);

	// all EOF lexemes, naturally, should
	// have the same position.
	assert(l->line == 1);
	assert(l->column == 10);
	assert(l->category == END_OF_FILE);

	lexeme_delete(l);
	sclose(s);
    });

    unit("lexing decimal integers", {
	stream *s = stream_fromstr("0    1  666");
	lexeme *l;

	l = scan(s);
	assert(l->category == INTEGER);
	assert(strcmp(l->content, "0") == 0);

	l = scan(s);
	assert(l->category == INTEGER);
	assert(strcmp(l->content, "1") == 0);

	l = scan(s);
	assert(l->category == INTEGER);
	assert(strcmp(l->content, "666") == 0);

	l = scan(s);
	assert(l->category == END_OF_FILE);

	lexeme_delete(l);
	sclose(s);
    });

    unit("lexing floating point numbers", {
	stream *s = stream_fromstr("0.43  0.0\n  5.23");
	lexeme *l;

	l = scan(s);
	assert(l->category == FLOAT);
	assert(strcmp(l->content, "0.43") == 0);

	l = scan(s);
	assert(l->category == FLOAT);
	assert(strcmp(l->content, "0.0") == 0);

	l = scan(s);
	assert(l->category == FLOAT);
	assert(strcmp(l->content, "5.23") == 0);

	l = scan(s);
	assert(l->category == END_OF_FILE);
	
	lexeme_delete(l);
	sclose(s);
    });

    unit("lexing hexadecimal integers", {
	stream *s = stream_fromstr("0x0 0xABCDEF \t 0X12");
	lexeme *l;

	l = scan(s);
	assert(l->category == HEX_INTEGER);
	assert(strcmp(l->content, "0x0") == 0);

	l = scan(s);
	assert(l->category == HEX_INTEGER);
	assert(strcmp(l->content, "0xABCDEF") == 0);

	l = scan(s);
	assert(l->category == HEX_INTEGER);
	assert(strcmp(l->content, "0X12") == 0);

	l = scan(s);
	assert(l->category == END_OF_FILE);
	
	lexeme_delete(l);
	sclose(s);
    });
})

int main() {
  test(stream);
  test(lexer);
  return 0;
}
