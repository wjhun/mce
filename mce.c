#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <runtime.h>

/*

  metacircular evaluator

  data types
  ----------

  cons type (pair)


  scheme primitives
  -----------------
  +
  >
  =
  *

  apply (but only for apply-primitive-procedure)
  and
  car
  cadr
  caddr
  cdr
  cond
  cons
  define
  else
  eq?
  error
  if
  lambda
  let
  list
  list-ref
  map
  null?
  number? (antiquated?)
  or
  pair?

  set-car!
  set-cdr!
  string?
  symbol?

 */

/* types

   value
   pair (of values)

   value types
   - empty list
   - pair
   - number
   - character
   - string
   - vector
   - symbol
   - procedure
 */


closure_function(0, 2, void, done,
                 value, v,
                 status, s)
{
    rprintf("parse finished: value %v, status %v\n", v, s);
    closure_finish();
}

static buffer read_stdin(heap h)
{
    buffer in = allocate_buffer(h, 1024);
    int r, k;
    while ((r = in->length - in->end) &&
           ((k = read(0, in->contents + in->end, r)), in->end += k, k > 0))
        buffer_extend(in, 1024);
    return in;
}

int main(int argc, char *argv[])
{
    heap h = init_process_runtime();

    parser p = sexp_parser(closure(h, done));

    // synchronous
    parser_feed(p, read_stdin(h));
    return EXIT_SUCCESS;
}
