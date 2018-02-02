/*
test.h
------
This file implements a preprocessor-only unit test
framework. It is inspired by, but significantly simpler
than, the one found at

https://mort.coffee/home/obscure-c-features/
*/

#pragma once

#ifndef __GNUC__
#error "GNU-extensions missing! Use clang or gcc."
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_MSG_MAX_LEN 256

/* ANSI escape codes for colours */
#define RED   "\x1B[31m"

#define GRN "\x1B[32m"

#define YLW "\x1B[33m"

#define BLU   "\x1B[34m"

#define MAG   "\x1B[35m"

#define CYN   "\x1B[36m"

#define WHT   "\x1B[37m"

#define RESET "\x1B[0m"


/* Unit Testing Macros */
#define assert(...)							\
  do {									\
    if (!(__VA_ARGS__)) {						\
      strcpy(message, "Assertion failed: "RED #__VA_ARGS__ RESET"\n");	\
      goto failure;							\
    }									\
  } while (0);


#define unit(description,...)						\
  do {									\
    __label__ failure;							\
    __label__ success;							\
    __label__ test_end;							\
    char *message = (char*) malloc(ERROR_MSG_MAX_LEN);			\
    __VA_ARGS__;							\
    goto success;							\
  failure:								\
    failed++;								\
    printf("%8s %-.40s"":\n", RED "Failure:" RESET, description);	\
    printf("    %s", message);						\
    printf("    In file: %s. Suite: %s\n", __FILE__, suite_name);	\
    goto test_end;							\
  success:								\
    printf("%8s %-40.40s %s: %s\n",					\
	   GRN "Success:" RESET,					\
	   description, __FILE__, suite_name);				\
  test_end:								\
    total++;								\
  } while (0);


#define suite(name,...)							\
  void test_##name() {							\
    const char* suite_name = #name;					\
    int failed = 0;							\
    int total = 0;							\
    printf("\nTesting " YLW "%s" RESET ":\n",				\
	   suite_name);							\
    __VA_ARGS__;							\
    printf(YLW "\n%s:" RESET " Passed %s%d/%d" RESET " tests.\n",	\
	   suite_name,							\
	   (failed == 0)? GRN : RED,					\
	   total - failed, total);					\
  }

#define test(name) test_##name()
