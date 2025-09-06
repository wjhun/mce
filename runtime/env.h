#pragma once

/* An env_frame represents a single frame in an environment as well as a whole
   environment (a chain of frames). */
typedef struct env_frame {
    table t;
    struct env_frame *next;
} *env_frame;

static inline env_frame enclosing_environment(env_frame e)
{
    return e->next;
}

static inline env_frame first_frame(env_frame e)
{
    return e;
}

static inline boolean is_empty_env(env_frame e)
{
    return !e;
}

env_frame extend_environment(pair variables, pair values, env_frame base_env);

value lookup_variable_value(symbol var, env_frame env);

boolean set_variable_value(symbol var, value val, env_frame env);

void define_variable(symbol var, value val, env_frame env);

void init_env(heap h, heap init);
