#include <stdio.h>
#include <string.h>

#include "test.h"

/*
 * Test Suite
 * ==========
 * Run the tests by running either
 *  a) make test
 * to just run the test, og
 *  b) make memcheck
 * to run the test through valgrind, which is preffered.
 * 
 * 
 */

#include "../ringbuffer.h"
#include "../lexer.h"
#include "../parser.h"
#include "../ast.h"

#include "suites/ringbuffer"
#include "suites/lexer"
#include "suites/parser"
#include "suites/ast"

int main() {
  test(ringbuffer);
  test(lexer);
  test(ast);
  test(parser);
  return 0;
}
