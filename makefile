all:
	clang -o parser lexer.c main.c stream.c

.PHONY: test
test:
	clang -o test/test lexer.c stream.c test/tmain.c
	cd test && ./test