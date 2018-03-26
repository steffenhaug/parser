debugbuild:
	gcc -o parser lexer.c main.c stream.c

productionbuild:
	gcc -o parser -O2 lexer.c main.c stream.c

.PHONY: test
test:
	gcc -O0 -o test/parser_test lexer.c stream.c test/tmain.c
	cd test && ./parser_test

.PHONY: macroexpansion
macroexpansion:
	gcc -E lexer.c main.c stream.c > macroe_xpansion
