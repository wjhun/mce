#include <runtime.h>
#include "mce.h"

symbol sym_true;
symbol sym_false;

typedef struct primitive_proc {
    const char *name;
    value (*fn)(value args);
} *primitive_proc;

static value prim_car(value args)
{
    return caar_of(args);
}

static value prim_cdr(value args)
{
    value v = car_of(args);
    return cdr_of(v);
}

static value prim_cons(value args)
{
    return cons(car_of(args), cadr_of(args));
}

static value prim_nullq(value args)
{
    return is_null(args) ? sym_false : sym_true;
}

static value prim_add(value args)
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

static value prim_gt(value args)
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

static value prim_eq(value args)
{
    value va = car_of(args);
    value vb = cadr_of(args);
    s64 a, b;
    if (!s64_from_value(va, &a)) {
        rprintf("'=': car not s64: %v\n", va);
        // exception
        return 0;
    }
    if (!s64_from_value(vb, &b)) {
        rprintf("'=': cadr not s64: %v\n", vb);
        // exception
        return 0;
    }
    return a == b ? sym_true : sym_false;
}

static value prim_mult(value args)
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

static value prim_list(value args)
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

env_frame primitive_environment(void)
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
