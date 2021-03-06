CFILES = ringbuffer.c lexer.c parser.c ast.c

debug:
	clang -o parser.out main.c $(CFILES)

prod:
	clang -o parser.out -O2 main.c $(CFILES)

.PHONY: test
test:
	clang -O0 -o test/parser_test.out -DLIE_DEBUG test/tmain.c $(CFILES)
	cd test && ./parser_test.out

.PHONY: memcheck
memcheck:
	clang -g -o test/parser_test.out -DLIE_DEBUG test/tmain.c $(CFILES)
	cd test && valgrind --leak-check=full --track-origins=yes ./parser_test.out

.PHONY: valgrind
valgrind:
	clang -o parser.out main.c $(CFILES)
	valgrind --leak-check=full --track-origins=yes ./parser.out
