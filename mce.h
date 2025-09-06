#pragma once

//#define MCE_DEBUG
#ifdef MCE_DEBUG
#define mce_debug(x, ...) do {rprintf("MCE %s: " x, __func__, ##__VA_ARGS__);} while(0)
#else
#define mce_debug(x, ...)
#endif

extern symbol sym_true;
extern symbol sym_false;

env_frame primitive_environment(void);

static inline boolean is_tagged_list(value exp, const char *tag)
{
    if (!is_pair(exp))
        return false;
    value car = car_of((pair)exp);
    return is_symbol(car) && sym_cstring_compare(car, tag);
}

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

static inline boolean is_let(value exp)
{
    return is_tagged_list(exp, "let");
}

static inline boolean self_evaluating(value exp)
{
    return is_integer(exp) || is_string(exp);
}

static inline boolean is_application(value exp)
{
    return is_pair(exp);
}

static inline value make_procedure(value params, value body, env_frame env)
{
    procedure p = compound_procedure(params, body, env);
    return list_from_args(2, sym(procedure), p);
}

static inline value make_lambda(value params, value body)
{
    return cons(sym(lambda), cons(params, body));
}
