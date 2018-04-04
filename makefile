CFILES = ringbuffer.c lexer.c parser.c ast.c

TMPLEXCHANGE = main.c ringbuffer.c lexer.c

lex:
	clang -o parser.out $(TMPLEXCHANGE)

debug:
	gcc -o parser.out main.c $(CFILES)

prod:
	gcc -o parser.out -O2 main.c $(CFILES)

.PHONY: test
test:
	gcc -O0 -o test/parser_test.out test/tmain.c $(CFILES)
	cd test && ./parser_test.out
