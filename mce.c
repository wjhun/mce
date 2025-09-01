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

static value m_eval(value exp, env_frame env);

static boolean self_evaluating(value exp)
{
    // XXX boolean
    return is_integer(exp) || is_string(exp);
}

static boolean is_tagged_list(value exp, symbol tag)
{
    if (!is_pair(exp))
        return false;
    value car = car_of((pair)exp);
    return is_symbol(car) && car == tag;
}

static inline boolean is_quoted(value exp)
{
    return is_tagged_list(exp, sym(quote));
}

static inline boolean is_assignment(value exp)
{
    return is_tagged_list(exp, sym_cstring("set!"));
}

static inline boolean is_definition(value exp)
{
    return is_tagged_list(exp, sym_cstring("define"));
}

static inline void eval_assignment(value exp, env_frame env)
{
    value var = cadr_of(exp); /* assignment-variable */
    value val = caddr_of(exp); /* assignment-value */
    rprintf("ass: var %v, val %p, env %p\n", var, val, env);
    if (!set_variable_value(var, val, env)) {
        // error path
        rprintf("Unbound variable -- SET! %v, env %p\n", var, env);
    }
}

static inline value definition_variable(value exp)
{
    value v = cadr_of(exp);
    if (is_symbol(v))
        return v;
    return caadr_of(exp);
}

static inline value definition_value(value exp)
{
    value v = cadr_of(exp);
    rprintf("%s: %v\n", __func__, v);
    if (is_symbol(v))
        return caddr_of(exp);
    // TODO lambdas
    // return make_lambda(cdadr_of(exp), cddr_of(exp));
    assert(0);
}

static inline void eval_definition(value exp, env_frame env)
{
    value defvar = definition_variable(exp);
    value val = definition_value(exp);
    rprintf("%s: var %v, value exp %v\n", __func__, defvar, val);
    val = m_eval(val, env);
    rprintf("after eval: %v\n", val);
    define_variable(defvar, val, env);
}

static value m_eval(value exp, env_frame env)
{
    rprintf("%s: exp %v, env %p\n", __func__, exp, env);
    if (self_evaluating(exp)) {
        return exp;
    } else if (is_symbol(exp)) { /* variable? */
        rprintf("lookup exp %v in env %p: ", exp, env);
        value v = lookup_variable_value((symbol)exp, env);
        rprintf("%p\n", v);
        return v;
    } else if (is_quoted(exp)) {
        return cadr_of(exp); /* text-of-quotation */
    } else if (is_assignment(exp)) {
        eval_assignment(exp, env);
        return 0;
    } else if (is_definition(exp)) {
        eval_definition(exp, env);
        return 0;
    }
    // if?
    // lambda?
    // begin?
    // cond?
    // application?
    // error "Unknown expression type -- EVAL" %v
    return 0;
}

closure_function(1, 2, void, done,
                 env_frame, env,
                 value, v, status, s)
{
    rprintf("parse finished: value %v, status %v\n", v, s);
    value result = m_eval(v, bound(env));
    rprintf("eval finished: result %v\n", result);
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

/* initial environment

   primitive-procedures list
   (list (list `car car)
        (list `cdr cdr)
        (list `cons cons)
        (list `null? null?)
        (list `+ +)
        (list `> >)
        (list `= =)
        (list `* *)
 */

static env_frame setup_initial_environment(void)
{
    pair p;
    p = cons(sym_cstring("*"), 0);
    p = cons(sym_cstring("="), p);
    p = cons(sym_cstring(">"), p);
    p = cons(sym_cstring("+"), p);
    p = cons(sym_cstring("null?"), p);
    p = cons(sym_cstring("cons"), p);
    p = cons(sym_cstring("cdr"), p);
    p = cons(sym_cstring("car"), p);
    rprintf("primitive names %v\n", p);

    // number placeholders
    pair q;
    q = cons(value_from_u64(8), 0);
    q = cons(value_from_u64(7), q);
    q = cons(value_from_u64(6), q);
    q = cons(value_from_u64(5), q);
    q = cons(value_from_u64(4), q);
    q = cons(value_from_u64(3), q);
    q = cons(value_from_u64(2), q);
    q = cons(value_from_u64(1), q);

    rprintf("primitive objects %v\n", q);

    return extend_environment(p, q, 0);
}

int main(int argc, char *argv[])
{
    heap h = init_process_runtime();

    env_frame env = setup_initial_environment();

    // synchronous
    do {
        parser p = sexp_parser(closure(h, done, env));
        parser_feed(p, read_stdin(h));
    } while (1);
    return EXIT_SUCCESS;
}
