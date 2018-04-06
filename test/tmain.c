#include <stdio.h>
#include <string.h>

#include "test.h"
#include "../lexer.h"
#include "../parser.h"
#include "../ast.h"

suite(Ringbuffer, { 
    // Verify that the source is not pointing to null.
    unit("create buffer from file", {
	ringbuffer b;
	init_filebuffer(&b, "ptest");

	assert(b.filename == "ptest");
	assert(b.source != NULL);
	assert(b.position == 0);
	assert(b.last_position != 0);
	assert(b.line == 1);
	assert(b.column == 0);

	free_ringbuffer(&b);
    });
    
    // Verify that the source IS pointing to null. 
    unit("create stream from string", {
	ringbuffer b;
	init_stringbuffer(&b, "");

	// source should be NULL, since the buffer
	// is not fo a file!
	assert(b.source == NULL);
	assert(b.position == 0);
	// last pos. should be 0, because the
	// string is empty!
	assert(b.last_position == 0);

	free_ringbuffer(&b);
    });
    
    unit("bgetch (from string)", {
	ringbuffer b;
	init_stringbuffer(&b, "abc");
	char x;

	bgetch(&b, &x);
	assert(x == 'a');
	bgetch(&b, &x);
	assert(x == 'b');
	bgetch(&b, &x);
	assert(x == 'c');

	free_ringbuffer(&b);
    });

    // if this suddenly broke, make sure the
    // file is still the same :-)
    unit("bgetch (from file)", {
	ringbuffer b;
	init_filebuffer(&b, "ptest");
	char x;

	bgetch(&b, &x);
	assert(x == '0');
	bgetch(&b, &x);
	assert(x == ' ');
	bgetch(&b, &x);
	assert(x == '1');

	free_ringbuffer(&b);
    });
    
    unit("empty buffer yields EOF forever", {
	ringbuffer b;
	init_stringbuffer(&b, "");
	char x;

	for (int i = 0; i < 25; i++) {
	  bgetch(&b, &x);
	  assert(x == EOF);
	}
	
	free_ringbuffer(&b);
    });
    
    unit("lookahead", {
	ringbuffer b;
	init_stringbuffer(&b, "abc");

	assert(look_ahead(&b, 0) == 'a');
	assert(look_ahead(&b, 1) == 'b');
	assert(look_ahead(&b, 2) == 'c');
	
	free_ringbuffer(&b);
    });
    
    unit("columns and lines (\"cursor\")", {
	ringbuffer b;
	init_stringbuffer(&b, "abc");
	char x;
	free_ringbuffer(&b);
    });

    unit("column and line does not move past EOF", {
	ringbuffer b;
	init_stringbuffer(&b,
			  "abc\n"
			  "def\n"
			  "ghi\n");

	char x;
	for (int j = 1; j <= 3; j++) {
	  for (int i = 1; i <= 4; i++) {
	    bgetch(&b, &x);
	    assert(b.column == i);
	    assert(b.line == j);
	  }
	}

	free_ringbuffer(&b);
    });

})

suite(Lexer, {
    unit("init/free lexeme", {
	lexeme l;
	init_lexeme(&l, LexNull, "", 1, 1);

	assert(l.type == LexNull);
	assert(l.line == 1);
	assert(l.column == 1);
	
	free_lexeme(&l);
    });

    unit("line/column tracking", {
	ringbuffer b;
	init_stringbuffer(&b,
			  "a b c \n"
			  "d e f \n"
			  "g h i \n");
	lexeme l;

	const char* expected_values[] = {
	  "a", "b", "c", "d", "e", "f", "g", "h", "i"
	}

	for (int j = 0; j < 3; j++) {
	  for (int i = 0; i < 3; i++) {
	    scan(&b, &l);

	    // verify content is right
	    assert(strcmp(l.content, expected_values[3 * j + i]) == 0);

	    // verify line/column is right
	    int expected_line = j + 1;
	    int expected_column = 1 + 2 * i;

	    assert(l.line == expected_line);
	    assert(l.column == expected_column);

	    free_lexeme(&l);
	  }
	}

	free_ringbuffer(&b);
    });

})


suite(Parser, {
})

suite(IR, {
    unit("initializing a root node", {
	ast root;
	init_ast(&root, ASTRoot);
	free_ast(&root);
    });

    unit("building minimal AST (+ 5 10) manually", {
	ast root;
	init_ast(&root, ASTRoot);

	ast plus, five, ten;
	init_ast(&five, ASTInteger);
	five.value.i = 5;

	init_ast(&ten, ASTInteger);
	ten.value.i = 0;

	init_ast(&plus, ASTPlus);
	push_child(&plus, five);
	push_child(&plus, ten);

	push_child(&root, plus);

	assert(root.children.data[0].type == ASTPlus);
	assert(root.children.data[0].children.data[0].type == ASTInteger);
	assert(root.children.data[0].children.data[1].type == ASTInteger);

	fit_ast_vector(&root.children);
	fit_ast_vector(&root.children.data[0].children);

	assert(root.children.length = 1);
	assert(root.children.data[0].children.length = 2);

	free_ast(&root); // should free five and ten recursively
    });

})

int main() {
  test(Ringbuffer);
  test(Lexer);
  test(IR);
  test(Parser);
  return 0;
}
