/* a meta-circular evaluator */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <runtime.h>

#include "mce.h"

#include <readline/readline.h>
#include <readline/history.h>

static value m_eval(value exp, env_frame env);

static void eval_assignment(value exp, env_frame env)
{
    value var = cadr_of(exp); /* assignment-variable */
    value val = caddr_of(exp); /* assignment-value */
    mce_debug("var %v, val %p, env %p\n", var, val, env);
    if (!set_variable_value(var, val, env)) {
        // error path
        rprintf("Unbound variable -- SET! %v, env %p\n", var, env);
    }
}

static value definition_variable(value exp)
{
    value v = cadr_of(exp);
    if (is_symbol(v))
        return v;
    return caadr_of(exp);
}

static value definition_value(value exp)
{
    value v = cadr_of(exp);
    if (is_symbol(v))
        return caddr_of(exp);
    return make_lambda(cdadr_of(exp), cddr_of(exp));
}

static void eval_definition(value exp, env_frame env)
{
    value defvar = definition_variable(exp);
    value val = definition_value(exp);
    mce_debug("var %v, value exp %v\n", defvar, val);
    val = m_eval(val, env);
    define_variable(defvar, val, env);
}

static value eval_if(value exp, env_frame env)
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
    return !is_null(exps) ? cons(m_eval(car_of(exps), env),
                                 list_of_values(cdr_of(exps), env)) : 0;
}

static value m_eval(value exp, env_frame env)
{
    mce_debug("exp %v, env %p\n", exp, env);
    if (self_evaluating(exp)) {
        return exp;
    } else if (is_symbol(exp)) { /* variable? */
        return lookup_variable_value((symbol)exp, env);
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

closure_function(2, 2, void, done,
                 env_frame, env, boolean *, complete,
                 value, v, status, s)
{
    mce_debug("parse finished: value %v, status %v\n", v, s);
    value result = m_eval(v, bound(env));
    if (!is_null(result)) {
        rprintf(is_string(result) ? "\"%v\"\n" : "%v\n", result);
    }
    *bound(complete) = true;
    closure_finish();
}

int main(int argc, char *argv[])
{
    heap h = init_process_runtime();
    env_frame env = primitive_environment();

    do {
        boolean complete = false;
        parser p = sexp_parser(closure(h, done, env, &complete));
        do {
            // this is actually lossy if an expression begins mid-line;
            // make the parser continuous
            char *input = readline("> ");
            if (!input) {
                // teardown
                return EXIT_SUCCESS;
            }
            if (*input) {
                p = parser_feed(p, alloca_wrap_cstring(input));
                /* newline needed for termination */
                if (p)
                    p = parser_feed(p, alloca_wrap_cstring("\n"));
                add_history(input);
            }
            free(input);
        } while (!complete);
    } while (1);
}
