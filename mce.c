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

//#define MCE_DEBUG
#ifdef MCE_DEBUG
#define mce_debug(x, ...) do {rprintf("MCE %s: " x, __func__, ##__VA_ARGS__);} while(0)
#else
#define mce_debug(x, ...)
#endif

symbol sym_true;
symbol sym_false;

static value m_eval(value exp, env_frame env);

static boolean self_evaluating(value exp)
{
    // XXX boolean
    return is_integer(exp) || is_string(exp);
}

static boolean is_tagged_list(value exp, const char *tag)
{
    if (!is_pair(exp))
        return false;
    value car = car_of((pair)exp);
    return is_symbol(car) && sym_cstring_compare(car, tag);
}

/* TODO cache these symbols */
static inline boolean is_quoted(value exp)
{
    return is_tagged_list(exp, "quote");
}

static inline boolean is_assignment(value exp)
{
    return is_tagged_list(exp, "set!");
}

static inline boolean is_definition(value exp)
{
    return is_tagged_list(exp, "define");
}

static inline boolean is_lambda(value exp)
{
    return is_tagged_list(exp, "lambda");
}

static inline boolean is_if(value exp)
{
    return is_tagged_list(exp, "if");
}

static inline boolean is_primitive_procedure(value exp)
{
    return is_tagged_list(exp, "primitive");
}

static inline boolean is_compound_procedure(value exp)
{
    return is_tagged_list(exp, "procedure");
}

static inline boolean is_begin(value exp)
{
    return is_tagged_list(exp, "begin");
}

static inline void eval_assignment(value exp, env_frame env)
{
    value var = cadr_of(exp); /* assignment-variable */
    value val = caddr_of(exp); /* assignment-value */
    mce_debug("var %v, val %p, env %p\n", var, val, env);
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

static inline value make_lambda(value params, value body)
{
    value cdr = cons(params, body);
    return cons(sym(lambda), cdr);
}

static inline value definition_value(value exp)
{
    value v = cadr_of(exp);
    if (is_symbol(v))
        return caddr_of(exp);
    return make_lambda(cdadr_of(exp), cddr_of(exp));
}

static inline void eval_definition(value exp, env_frame env)
{
    value defvar = definition_variable(exp);
    value val = definition_value(exp);
    mce_debug("var %v, value exp %v\n", defvar, val);
    val = m_eval(val, env);
    mce_debug("after eval: %v\n", val);
    define_variable(defvar, val, env);
}

static inline value make_procedure(value params, value body, env_frame env)
{
    procedure p = compound_procedure(params, body, env);
    return list_from_args(2, sym(procedure), p);
}

static inline value eval_if(value exp, env_frame env)
{
    value predicate_exp = cadr_of(exp);
    value predicate_value = m_eval(predicate_exp, env);
    mce_debug("pred exp %v, value %v\n", predicate_exp, predicate_value);

    if (predicate_value != sym_false) {
        value consequent_exp = caddr_of(exp);
        value consequent_value = m_eval(consequent_exp, env);
        mce_debug("   conseq exp %v, value %v\n", consequent_exp, consequent_value);
        return consequent_value;
    }
    value alt_exp = !is_null(cdddr_of(exp)) ? cadddr_of(exp) : sym_false;
    value alt_value = m_eval(alt_exp, env);
    mce_debug("   alt exp %v, value %v\n", alt_exp, alt_value);
    return alt_value;
}

static inline boolean is_application(value exp)
{
    return is_pair(exp);
}

static value eval_sequence(value seq, env_frame env)
{
    value result = 0;
    while (is_pair(seq)) {
        value car = car_of(seq);
        result = m_eval(car, env);
        seq = cdr_of(seq);
    }
    return result;
}

static value m_apply(value proc, value args)
{
    if (is_primitive_procedure(proc)) {
        /* apply-primitive-procedure */
        procedure impl = cadr_of(proc);
        assert(is_procedure((value)impl));
        assert(procedure_is_primitive(impl)); /* internal state agrees */
        value result = impl->fn(args); /* apply */
        mce_debug("primitive %v, args %v, result %v\n", impl, args, result);
        return result;
    }

    if (is_compound_procedure(proc)) {
        procedure impl = cadr_of(proc);
        assert(is_procedure((value)impl));
        assert(!procedure_is_primitive(impl)); /* internal state agrees */

        env_frame extended_env = extend_environment(impl->params, args, impl->env);
        value result = eval_sequence(impl->body, extended_env);
        mce_debug("compound body %v, params %v, args %v, result %v\n",
                  impl->body, impl->params, args, result);
        return result;
    }

    rprintf("Unknown procedure type -- APPLY %v\n", proc);
    return 0;
}

static value list_of_values(value exps, env_frame env)
{
    return exps ? cons(m_eval(car_of(exps), env),
                       list_of_values(cdr_of(exps), env)) : 0;
}

static value m_eval(value exp, env_frame env)
{
    mce_debug("exp %v, env %p\n", exp, env);
    if (self_evaluating(exp)) {
        return exp;
    } else if (is_symbol(exp)) { /* variable? */
        value v = lookup_variable_value((symbol)exp, env);
        return v;
    } else if (is_quoted(exp)) {
        return cadr_of(exp); /* text-of-quotation */
    } else if (is_assignment(exp)) {
        eval_assignment(exp, env);
        return 0;
    } else if (is_definition(exp)) {
        eval_definition(exp, env);
        return 0;
    } else if (is_if(exp)) {
        return eval_if(exp, env);
    } else if (is_lambda(exp)) {
        value params = cadr_of(exp);
        value body = cddr_of(exp);
        return make_procedure(params, body, env);
    } else if (is_begin(exp)) {
        value actions = cdr_of(exp);
        return eval_sequence(actions, env);
    }
    // cond?
    else if (is_application(exp)) {
        value proc = m_eval(car_of(exp) /* operator */, env);
        value args = list_of_values(cdr_of(exp) /* operands */, env);
        return m_apply(proc, args);
    } else {
        rprintf("Unknown expression type -- EVAL %v\n", exp);
    }
    return 0;
}

closure_function(1, 2, void, done,
                 env_frame, env,
                 value, v, status, s)
{
    mce_debug("parse finished: value %v, status %v\n", v, s);
    value result = m_eval(v, bound(env));
    rprintf("%v\n", result);
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

typedef struct primitive_proc {
    const char *name;
    value (*fn)(value args);
} *primitive_proc;

value prim_car(value args)
{
    return car_of(args);
}

value prim_cdr(value args)
{
    return cdr_of(args);
}

value prim_cons(value args)
{
    return cons(car_of(args), cadr_of(args));
}

value prim_nullq(value args)
{
    return is_null(args) ? sym_false : sym_true;
}

value prim_add(value args)
{
    value va = car_of(args);
    value vb = cadr_of(args);
    s64 a, b, c;
    if (!s64_from_value(va, &a)) {
        rprintf("'+': car not s64: %v\n", va);
        // exception
        return 0;
    }
    if (!s64_from_value(vb, &b)) {
        rprintf("'+': cadr not s64: %v\n", vb);
        // exception
        return 0;
    }
    c = a + b;
    return value_from_s64(c);
}

value prim_gt(value args)
{
    value va = car_of(args);
    value vb = cadr_of(args);
    s64 a, b;
    if (!s64_from_value(va, &a)) {
        rprintf("'>': car not s64: %v\n", va);
        // exception
        return 0;
    }
    if (!s64_from_value(vb, &b)) {
        rprintf("'>': cadr not s64: %v\n", vb);
        // exception
        return 0;
    }
    return a > b ? sym_true : sym_false;
}

value prim_eq(value args)
{
    value va = car_of(args);
    value vb = cadr_of(args);
    return is_symbol(va) && is_symbol(vb) && va == vb ? sym_true : sym_false;
}

value prim_mult(value args)
{
    value va = car_of(args);
    value vb = cadr_of(args);
    s64 a, b, c;
    if (!s64_from_value(va, &a)) {
        rprintf("'*': car not s64: %v\n", va);
        // exception
        return 0;
    }
    if (!s64_from_value(vb, &b)) {
        rprintf("'*': cadr not s64: %v\n", vb);
        // exception
        return 0;
    }
    c = a * b;
    return value_from_s64(c);
}

value prim_list(value args)
{
    if (is_null(args))
        return 0;
    return cons(car_of(args), prim_list(cdr_of(args)));
}

static struct primitive_proc primitive_procs[] = {
    { "car", prim_car },
    { "cdr", prim_cdr },
    { "cons", prim_cons },
    { "null?", prim_nullq },
    { "+", prim_add },
    { ">", prim_gt },
    { "=", prim_eq },
    { "*", prim_mult },
    { "list", prim_list },
    { 0, 0 }
};

static pair cons_prim_names(primitive_proc p)
{
    return p->name ? cons(sym_cstring(p->name), cons_prim_names(p + 1)) : 0;
}

static pair cons_prim_objects(primitive_proc p)
{
    if (!p->name)
        return 0;
    value obj = list_from_args(2, sym(primitive), primitive_procedure(p->fn));
    return cons(obj, cons_prim_objects(p + 1));
}

static env_frame setup_initial_environment(void)
{
    /* primitive-procedure-names */
    pair names = cons_prim_names(primitive_procs);
    mce_debug("primitive names %v\n", names);

    /* primitive-procedure-objects */
    pair objs = cons_prim_objects(primitive_procs);
    mce_debug("primitive objects %p\n", objs);

    sym_true = sym_cstring("true");
    sym_false = sym_cstring("false");
    return extend_environment(names, objs, 0);
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
