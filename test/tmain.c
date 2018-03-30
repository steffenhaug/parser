#include <stdio.h>
#include <string.h>

#include "test.h"
#include "../lexer.h"
#include "../stream.h"
#include "../parser.h"
#include "../ast.h"

suite(Stream, { 
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


#define verify(str, kw)					\
    l = scan(s);					\
    assert(l->type == kw);				\
    assert(strcmp(l->content, str) == 0)

suite(Lexer, {
    unit("create and delete lexeme", {
	lexeme *l = lexeme_new(LexEndOfFile, "eof", 1, 1);
	assert(l != NULL);
	
	// check that no members are left uninitialized
	assert(l->type == LexEndOfFile);
	assert(strcmp(l->content, "eof") == 0);
	assert(l->line == 1);
	assert(l->column == 1);

	// delete it
	free_lexeme(l);
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
	assert(l->type == LexEndOfFile);

	// try to advance past EOF
	l = scan(s);
	l = scan(s);

	// all EOF lexemes, naturally, should
	// have the same position.
	assert(l->line == 1);
	assert(l->column == 10);
	assert(l->type == LexEndOfFile);

	free_lexeme(l);
	sclose(s);
    });

    unit("lexing decimal integers", {
	stream *s = stream_fromstr("0    1  666");
	lexeme *l;

	l = scan(s);
	assert(l->type == LexDecInteger);
	assert(strcmp(l->content, "0") == 0);

	l = scan(s);
	assert(l->type == LexDecInteger);
	assert(strcmp(l->content, "1") == 0);

	l = scan(s);
	assert(l->type == LexDecInteger);
	assert(strcmp(l->content, "666") == 0);

	l = scan(s);
	assert(l->type == LexEndOfFile);

	free_lexeme(l);
	sclose(s);
    });

    unit("lexing floating point numbers", {
	stream *s = stream_fromstr("0.43  0.0\n  5.23");
	lexeme *l;

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "0.43") == 0);

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "0.0") == 0);

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "5.23") == 0);

	l = scan(s);
	assert(l->type == LexEndOfFile);
	
	free_lexeme(l);
	sclose(s);
    });

    unit("lexing hex and binary integers", {
	stream *s = stream_fromstr("0x0 0xABCDEF \t 0X12 0b101 0B01110");
	lexeme *l;

	l = scan(s);
	assert(l->type == LexHexInteger);
	assert(strcmp(l->content, "0x0") == 0);

	l = scan(s);
	assert(l->type == LexHexInteger);
	assert(strcmp(l->content, "0xABCDEF") == 0);

	l = scan(s);
	assert(l->type == LexHexInteger);
	assert(strcmp(l->content, "0X12") == 0);

	l = scan(s);
	assert(l->type == LexBinInteger);
	assert(strcmp(l->content, "0b101") == 0);

	l = scan(s);
	assert(l->type == LexBinInteger);
	assert(strcmp(l->content, "0B01110") == 0);

	l = scan(s);
	assert(l->type == LexEndOfFile);
	
	free_lexeme(l);
	sclose(s);
    });

    unit("lexing scientific notation", {
	stream *s = stream_fromstr("1e1 1.2e2 1e-2 1e0 1e5.");
	lexeme *l;

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "1e1") == 0);

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "1.2e2") == 0);

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "1e-2") == 0);

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "1e0") == 0);

	l = scan(s);
	assert(l->type == LexFloat);
	assert(strcmp(l->content, "1e5") == 0);

	l = scan(s);
	assert(l->type == LexDot);

	l = scan(s);
	assert(l->type == LexEndOfFile);

	free_lexeme(l);
	sclose(s);
    });

    unit("lexing operators and special symbols", {
	stream *s = stream_fromstr("+ - * / . ... = == != < > <= >= "
				   ": ; , -> <-");
	lexeme *l;

	l = scan(s);
	assert(l->type == LexPlus);
	l = scan(s);
	assert(l->type == LexMinus);
	l = scan(s);
	assert(l->type == LexAsterisk);
	l = scan(s);
	assert(l->type == LexSlash);
	l = scan(s);
	assert(l->type == LexDot);
	l = scan(s);
	assert(l->type == LexEllipsis);
	l = scan(s);
	assert(l->type == LexEquals);
	l = scan(s);
	assert(l->type == LexDoubleEquals);
	l = scan(s);
	assert(l->type == LexNotEqual);
	l = scan(s);
	assert(l->type == LexLessThan);
	l = scan(s);
	assert(l->type == LexGreaterThan);
	l = scan(s);
	assert(l->type == LexLessOrEq);
	l = scan(s);
	assert(l->type == LexGreaterOrEq);
	// line break
	l = scan(s);
	assert(l->type == LexColon);
	l = scan(s);
	assert(l->type == LexSemicolon);
	l = scan(s);
	assert(l->type == LexComma);
	l = scan(s);
	assert(l->type == LexRightArrow);
	l = scan(s);
	assert(l->type == LexLeftArrow);


	l = scan(s);
	assert(l->type == LexEndOfFile);

	free_lexeme(l);
	sclose(s);
    });

    unit("lexing brackets", {
	stream *s = stream_fromstr("() {} [] <>");
	lexeme *l;

	l = scan(s);
	assert(l->type == LexLeftParenthesis);
	l = scan(s);
	assert(l->type == LexRightParenthesis);

	l = scan(s);
	assert(l->type == LexLeftCurlyBrace);
	l = scan(s);
	assert(l->type == LexRightCurlyBrace);

	l = scan(s);
	assert(l->type == LexLeftSquareBracket);
	l = scan(s);
	assert(l->type == LexRightSquareBracket);

	l = scan(s);
	assert(l->type == LexLessThan);
	l = scan(s);
	assert(l->type == LexGreaterThan);

	free_lexeme(l);
	sclose(s);
    });

    unit("lex segment with comment", {
	stream *s = stream_fromstr("->"
				   "// foobar loodar. \n"
				   "// Comments should be invisible \n"
				   "// past the lexing stage, \n"
				   "// thus the lexer should only \n"
				   "// emit two RightArrows (->) for \n"
				   "// this segment of text \n"
				   "->");
	lexeme *l;

	l = scan(s);
	assert(l->type == LexRightArrow);
	l = scan(s);
	assert(l->type == LexRightArrow);

	free_lexeme(l);
	sclose(s);
    });

    unit("lex strings", {
	stream *s = stream_fromstr(" \"foobar\" \"loodar\" ");
	lexeme *l;

	verify("foobar", LexString);
	verify("loodar", LexString);
	
	free_lexeme(l);
	sclose(s);
    });

    unit("lex identifiers", {
	stream *s = stream_fromstr("foobar foo123 loodar?");
	lexeme *l;

	verify("foobar", LexIdentifier);
	verify("foo123", LexIdentifier);
	verify("loodar?", LexIdentifier);

	free_lexeme(l);
	sclose(s);
    });

    unit("lex keywords", {
	stream *s = stream_fromstr("mod div func fn use as "
				   "let in if else switch default "
				   "cases otherwise and or xor true false ");
	lexeme *l;

	verify("mod", LexMod);
	verify("div", LexDiv);
	verify("func", LexFunc);
	verify("fn", LexFn);
	verify("use", LexUse);
	verify("as", LexAs);
	verify("let", LexLet);
	verify("in", LexIn);
	verify("if", LexIf);
	verify("else", LexElse);
	verify("switch", LexSwitch);
	verify("default", LexDefault);
	verify("cases", LexCases);
	verify("otherwise", LexOtherwise);
	verify("and", LexAnd);
	verify("or", LexOr);
	verify("xor", LexXor);
	verify("true", LexTrue);
	verify("false", LexFalse);

	free_lexeme(l);
	sclose(s);
    });
})
#undef verify


suite(Parser, {
    unit("match, LA, LT", {
	// LT uses LA under the hood, so i mostly test LT
	stream *s = stream_fromstr("666 + 42 * x");
	parser p;
	init_parser(&p, s);

	if (0) // Print lookahead buffer.
	  for (int i = 0; i < MAX_LOOKAHEAD; i++) {
	    lexeme *a = p.lookahead[i];
	    printf("(lexeme) %-20s %10s (line %2d, column %2d)\n",
		   lexeme_class_tostr(a->type), a->content, a->line, a->column);
	  }

	assert(LT(&p, 0) == LexDecInteger);
	assert(strcmp(LA(&p, 0)->content, "666") == 0);
	assert(LA(&p, 0)->line == 1);
	assert(LA(&p, 0)->column == 1);
	assert(LA(&p, 0)->type == LexDecInteger);
	
	assert(LT(&p, 1) == LexPlus);
	assert(LT(&p, 2) == LexDecInteger);
	assert(LT(&p, 3) == LexAsterisk);
	assert(LT(&p, 4) == LexIdentifier);

	match(&p, LexDecInteger);
	assert(LT(&p, 0) == LexPlus);
	assert(LT(&p, 1) == LexDecInteger);
	assert(LT(&p, 2) == LexAsterisk);
	assert(LT(&p, 3) == LexIdentifier);

	match(&p, LexPlus);
	assert(LT(&p, 0) == LexDecInteger);
	assert(LT(&p, 1) == LexAsterisk);
	assert(LT(&p, 2) == LexIdentifier);

	match(&p, LexDecInteger);
	assert(LT(&p, 0) == LexAsterisk);
	assert(LT(&p, 1) == LexIdentifier);

	match(&p, LexAsterisk);
	assert(LT(&p, 0) == LexIdentifier);

	match(&p, LexIdentifier);

	match(&p, LexEndOfFile);
  
	sclose(s);
    });

    unit("parse empty stream", {
	stream *s = stream_fromstr("");
	parser p;
	init_parser(&p, s);

	ast root;
	parse_root(&p, &root);

	assert(root.type == ASTRoot);
	assert(root.value.i == 0);

	// Nothing is even malloced if the stream is empty
	assert(root.children.length == 0);
	assert(root.children.capacity == 0);
	assert(root.children.data == NULL);

	assert(root.span.start_line ==
	       root.span.end_line == 1);

	assert(root.span.start_column ==
	       root.span.end_column == 1);

	sclose(s);
    });

    unit("parse atoms", {
	stream *s = stream_fromstr("5 0xFF 0b10 0.42 \"f o o b a r\" x true false");
	parser p;
	init_parser(&p, s);

	ast atom;
	int error_code = 0;

	// 5
	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTInteger);
	assert(atom.value.i == 5);

	// 0xFF
	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTInteger);
	assert(atom.value.i == 255);

	// 0b10
	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTInteger);
	assert(atom.value.i == 2);

	// 0.42
	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTFloat);
	assert(atom.value.d == 0.42);

	// "f o o b a r"
	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTString);
	assert(strcmp(atom.value.s, "f o o b a r") == 0);

	// x (identifier)
	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTIdentifier);
	assert(strcmp(atom.value.s, "x") == 0);

	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTBool);
	assert(atom.value.b == true);

	error_code = parse_atom(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTBool);
	assert(atom.value.b == false);
    });

    unit("parse primary expr", {
	stream *s = stream_fromstr("2.3 4.5e6");
	parser p;
	init_parser(&p, s);
	
	ast atom;
	int error_code = 0;

	error_code = parse_primary_expr(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTFloat);
	assert(atom.value.d == 2.3);

	error_code = parse_primary_expr(&p, &atom);
	assert(!error_code);
	assert(atom.type == ASTFloat);
	assert(atom.value.d == atof("4.5e6"));


	printf("remember: atof(\"4.5e6.7\") = %.3f, so the elxer needs a change\n",
	       atom.value.d);
	printf("remember to test function and subscript "
	       "primary expressions as well.\n");
    });

    unit("parse factors", {
	stream *s = stream_fromstr("-a -5 3^2");
	parser p;
	init_parser(&p, s);

	ast n;
	int error_code = 0;

	error_code = parse_factor(&p, &n);
	assert(!error_code);
	assert(n.type == ASTUnaryMinus);

	// notice! negative integers are lexed as integers,
	// so "-5" is not a unary expression.
	error_code = parse_factor(&p, &n);
	assert(!error_code);
	assert(n.type == ASTInteger); // !!!
	assert(n.value.i = -5);

	error_code = parse_factor(&p, &n);
	assert(!error_code);
	assert(n.type == ASTPow); // !!!
	assert(n.value.i = -5);
    });

    unit("parse power expressions", {
	stream *s = stream_fromstr("2^3 2.6^12 2^-3.4");
	parser p;
	init_parser(&p, s);

	ast n;
	int error_code = 0;

	error_code = parse_power(&p, &n);
	assert(!error_code);
	assert(n.type == ASTPow);

	error_code = parse_power(&p, &n);
	assert(!error_code);
	assert(n.type == ASTPow);

	error_code = parse_power(&p, &n);
	assert(!error_code);
	assert(n.type == ASTPow);
    });

    unit("parse terms", {
	stream *s = stream_fromstr("2 * 3       4 / 2     15 mod 4");
	parser p;
	init_parser(&p, s);

	ast n;
	int error_code = 0;

	error_code = parse_term(&p, &n);
	assert(!error_code);
	assert(n.type == ASTMul);

	error_code = parse_term(&p, &n);
	assert(!error_code);
	assert(n.type == ASTDiv);

	error_code = parse_term(&p, &n);
	assert(!error_code);
	assert(n.type == ASTMod);

    });

    unit("parse arithmetic", {
	stream *s = stream_fromstr("2 + 6 * 7 - 4 mod 6");
	//                   terms: -   -----   -------
	parser p;
	init_parser(&p, s);

	ast tree;
	int error_code = 0;

	error_code = parse_arith_expr(&p, &tree);
	assert(!error_code);
	assert(tree.type = ASTMinus);
	assert(tree.children.data[0].type == ASTPlus);
	assert(tree.children.data[0].children.data[1].type == ASTMul);
	assert(tree.children.data[1].type == ASTMod);
    });

    unit("parse comparisons", {
	stream *s = stream_fromstr("a < b     x < 5 < y   ");
	parser p;
	init_parser(&p, s);

	ast tree;
	int error_code = 0;

	error_code = parse_comp_expr(&p, &tree);
	assert(!error_code);

	assert(tree.type == ASTComp);
	assert(tree.children.data[0].type == ASTIdentifier);
	assert(tree.children.data[1].type == ASTCompOps);
	assert(tree.children.data[2].type == ASTCompOperands);

	assert(tree.children.data[1].children.length == 1);
	assert(tree.children.data[2].children.length == 1);

	assert(tree.children.data[1].children.data[0].type == ASTLess);
	assert(tree.children.data[2].children.data[0].type == ASTIdentifier);

	error_code = parse_comp_expr(&p, &tree);
	assert(!error_code);

	assert(tree.children.data[0].type == ASTIdentifier);
	assert(tree.children.data[1].type == ASTCompOps);
	assert(tree.children.data[2].type == ASTCompOperands);

	assert(tree.children.data[1].children.length == 2);
	assert(tree.children.data[2].children.length == 2);
    });

    unit("parse not statements", {
	stream *s = stream_fromstr("true     not true    not false ");
	parser p;
	init_parser(&p, s);

	ast tree;
	int error_code = 0;

	error_code = parse_not_expr(&p, &tree);
	assert(!error_code);
	assert(tree.type == ASTBool);

	error_code = parse_not_expr(&p, &tree);
	assert(!error_code);
	assert(tree.type == ASTNot);
	assert(tree.children.data[0].type == ASTBool);
	assert(tree.children.data[0].value.b == true);

	error_code = parse_not_expr(&p, &tree);
	assert(!error_code);
	assert(tree.type == ASTNot);
	assert(tree.children.data[0].type == ASTBool);
	assert(tree.children.data[0].value.b == false);
    });

    unit("parse and expressions", {
	stream *s = stream_fromstr("a and b   not a and b   a and not b");
	parser p;
	init_parser(&p, s);

	ast tree;
	int error_code = 0;

	error_code = parse_and_expr(&p, &tree);
	assert(!error_code);
	assert(tree.type == ASTAnd);
	assert(tree.children.data[0].type == ASTIdentifier);
	assert(tree.children.data[1].type == ASTIdentifier);

	error_code = parse_and_expr(&p, &tree);
	assert(!error_code);
	assert(tree.type == ASTAnd);
	assert(tree.children.data[0].type == ASTNot);
	assert(tree.children.data[1].type == ASTIdentifier);

	error_code = parse_and_expr(&p, &tree);
	assert(!error_code);
	assert(tree.type == ASTAnd);
	assert(tree.children.data[0].type == ASTIdentifier);
	assert(tree.children.data[1].type == ASTNot);
    });

    unit("parse or (and xor) exprs", {
	stream *s = stream_fromstr("a or b     a xor b");
	parser p;
	init_parser(&p, s);

	ast tree;
	int error_code = 0;

	error_code = parse_or_expr(&p, &tree);
	assert(!error_code);

	error_code = parse_or_expr(&p, &tree);
	assert(!error_code);
    });
    
    unit("autoconstructing ast from '5 + 10.'", {
	stream *s = stream_fromstr("5 + 10.");
	parser p;
	init_parser(&p, s);

	ast root;
	int error_code = parse_root(&p, &root);
	assert(!error_code);

	assert(root.children.data[0].type == ASTPlus);
	assert(root.children.data[0].children.data[0].type == ASTInteger);
	assert(root.children.data[0].children.data[1].type == ASTInteger);
    });
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

	free_ast(&root);
    });

})

int main() {
  test(Stream);
  test(Lexer);
  test(IR);
  test(Parser);
  return 0;
}
