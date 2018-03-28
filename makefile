CFILES = stream.c lexer.c parser.c ast.c

debug:
	gcc -o parser main.c $(CFILES)

prod:
	gcc -o parser -O2 main.c $(CFILES)

.PHONY: test
test:
	gcc -O0 -o test/parser_test test/tmain.c $(CFILES)
	cd test && ./parser_test
