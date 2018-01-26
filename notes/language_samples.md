# Syntax 

```
-- a comment
-- and another one

---
multi
line
comment
---

-- standard library imports
use <set>.
use <hashmap>.

-- your own imports
use another_file.
use module/from_module.


-- symbol definitions

let pi = 3.1415.
let x = pi * 5.

-- functions

fun zero? n: n = 0.

fun fac n:
  if zero?(n): 1.
  else:
    n * fac(n - 1).

fun even? n:
  n mod 2 = 0.

fun odd? n:
  n mod 2 = 1.


-- structures

struct List {
  payload,
  list
}.

struct List: payload, list.

List(5, List(6, List(7, List (8, List(9, NIL)))))



```