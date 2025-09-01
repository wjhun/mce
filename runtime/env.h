#pragma once

/* represents a single frame in an environment - but also is handle to the
   environment expressed by the chain of frames starting with the frame
   pointed to */

typedef struct env_frame {
    table t;
    struct env_frame *next;
} *env_frame;

/*** users of environments: ***/

// enclosing-environment

static inline env_frame enclosing_environment(env_frame e)
{
    return e->next;
}

// first-frame
static inline env_frame first_frame(env_frame e)
{
    return e;
}

// the-empty-environment

/* wrap this in case we want another singleton as empty env */
static inline boolean is_empty_env(env_frame e)
{
    return !e;
}

env_frame extend_environment(pair variables, pair values, env_frame base_env);

static inline value lookup_variable_value(symbol var, env_frame env)
{
    while (!is_empty_env(env)) {
        value v = (value)table_find(env->t, var);
        if (v)
            return v;
        env = env->next;
    }
    return 0;
}

/* overwrite existing value only, return false if not found */
static inline boolean set_variable_value(symbol var, value val, env_frame env)
{
    while (!is_empty_env(env)) {
        value v = (value)table_find(env->t, var);
        if (v) {
            rprintf("%s: t %p var %v -> val %p\n", __func__, env->t, var, val);
            table_set(env->t, var, val);
            return true;
        }
        env = env->next;
    }
    return false;
}

// XXX add scan
static inline void define_variable(symbol var, value val, env_frame env)
{
    assert(env);
    rprintf("%s: t %p, var %v, val %p\n", __func__, env, var, val);
    table_set(env->t, var, val);
}

void init_env(heap h, heap init);

/*** eval ***/

// m-eval

// eval-assignment

// eval-definition

// eval-if

// eval-sequence

// list-of-values


