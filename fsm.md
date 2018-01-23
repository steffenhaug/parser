Finite State Machines
---------------------

FSMs will probably be used for lexing.

DFSM:
Sigma	input alphabet
S	finite set of states
s0	start state
delta	transition function Sigma x S -> S
E	set of exit states

Finite state transducer:
Sigma	input alphabet
Gamma	output alphabet
S	finite set of states
s0	start state
delta	transition function. Sigma x S -> S
omega	output function. Sigma x S -> Gamma


the lexer will probably be more like a DFSM,
with a STACK to put the currently lexed string on.
the mexhine will output lexeme structs, but not on every
transition, so a FST is not necesarily the optimal
structure.

Sigma	  set of all valid characters in source file (in practice unicode)
	  NOTE: Sigma may be a morphism on input characters, categorizing
	  them in equivalence classes.
	  class('a' .. 'z') -> LOWERCASE_ALPHA
	  class('A' .. 'Z') -> UPPERCASE_ALPHA
	  class('0' .. '9') -> DIGIT

S	  all lexing states, like SCANNING_NUMBER, SCANNING_STRING,
	  SCANNING_IDENTIFIER,
	  
s0	  basic SCAN state, nothing on stack yet

delta 	  switch on current character. in the case of numbers:
	     0,	    SCAN       	  -> SEEN_ZERO
	     DIGIT, SCAN       	  -> SCAN_DEC
	     0,     SEEN_ZERO  	  -> SEEN_ZERO
	     x,     SEEN_ZERO  	  -> SCAN_HEX
	     b,     SEEN_ZERO  	  -> SCAN_BIN
	     .,	    SEEN_ZERO  	  -> SCAN_FLOAT
	     .,	    SCAN_DEC 	  -> SCAN_FLOAT
	     e,	    SCAN_DEC	  -> SCAN_EXP
	     e,	    SCAN_FLOAT	  -> SCAN_EXP
	     DIGIT, SEEN_ZERO  	  -> SCAN_DEC
	     _,     SEEN_ZERO  	  -> EXIT		=> lexeme {DEC_INTEGER, "0", l, c}
	     _,	    SCAN_HEX	  -> EXIT 		=> lexeme {HEX_INTEGER, buf, l, c}
	     _,	    SCAN_BIN	  -> EXIT	 	=> lexeme {BIN_INTEGER, buf, l, c}
	     _,	    SCAN_DEC	  -> EXIT 		=> lexeme {DEC_INTEGER, buf, l, c}
	     _,	    SCAN_FLOAT	  -> EXIT 		=> lexeme {FLOAT, buf, l, c}
	     _,	    SCAN_EXP	  -> EXIT 		=> lexeme {EXP, buf, l, c}
	     
	  first column is values for Σ (alphabet/equivalence-classes)
	  second column is values for S (states)
	  third column is values for delta: Σ x S -> delta (the next state)
	  forth column is the output values.

e	  the EXIT state will probably just push back the last character,
	  reset the stack, and build the lexeme.


### the big question
how to structure the automata efficiently?

the most efficient option, presumably, is to hand-code the transition table
with switches and gotos. this will make for a very large function.


### main lexing routine
```
int state = START;
static int l = 0;
static int c = 0;

exit:
// push back the char that identified the exit
push_back_char(stack[stackp]);
// null-terminate the lexeme content
stack[stackp] = '\0';
// build a lexeme-structure
lexeme *t = malloc(sizeof(lexeme));
t->category = cat(state);
strcpy(t->content, stack)
t->line = l;
t->column = c;
return t;

// stack to build the lexeme
char stack[STACK_DEPTH];
// -1 is the default in start-mode
int stackp = -1;

do {
   stack[++stackp] = next_char()
   switch (stack[stackp]) {
       
   }


} while (stack[stackp] != EOF)
```

# some useful equivalence classes:
WHITESPACE:   		  \t|space
LINE_BREAK:		  \n|\r

DIGIT: 	      		  0-9
HEX_DIGIT:		  a-fA-F0-9
ALPHA_LCASE: 		  a-z
ALPHA_UCAPS: 		  A-Z
ALPHA: 			  a-zA-Z
ALPHANUMERIC: 		  a-zA-Z0-9

### implementation of equivalence classes:
function with switch that returns an enumeration, and
rewrite scan to switch on these enums.

macros:

case CLASS_DIGIT:
  ...


will expand co

case '0':
case '1':
case '2':
...
case '9':
  ...



if "CLASS_DIGIT" is a macro:

#define CLASS_DIGIT \
'0'\
case '1':\
case '2':\
case '3':\
...
case '9':\

tis is a bit nasty, as the macro expands to a broken case clause, but
in the source it would look nice.










