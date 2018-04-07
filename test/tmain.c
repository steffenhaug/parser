#include <stdio.h>
#include <string.h>

#include "test.h"

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
