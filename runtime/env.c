#include <runtime.h>

static heap env_heap;

value lookup_variable_value(symbol var, env_frame env)
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
boolean set_variable_value(symbol var, value val, env_frame env)
{
    while (!is_empty_env(env)) {
        value v = (value)table_find(env->t, var);
        if (v) {
            // rprintf("%s: t %p var %v -> val %p\n", __func__, env->t, var, val);
            table_set(env->t, var, val);
            return true;
        }
        env = env->next;
    }
    return false;
}

void define_variable(symbol var, value val, env_frame env)
{
    assert(env);
    // rprintf("%s: t %p, var %v, val %p\n", __func__, env, var, val);
    table_set(env->t, var, val);
}

env_frame extend_environment(pair variables, pair values, env_frame base_env)
{
    int n = pair_list_length(variables);
    int m = pair_list_length(values);

    // error path?
    if (n != m) {
        msg_err("variable, value list length mismatch (%d, %d)\n", n, m);
        return INVALID_ADDRESS;
    }

    table t = allocate_table(env_heap, key_from_symbol, pointer_equal);
    if (t == INVALID_ADDRESS)
        return INVALID_ADDRESS;
    env_frame ef = allocate(env_heap, sizeof(*ef));
    if (ef != INVALID_ADDRESS) {
        ef->t = t;
        ef->next = base_env;
    }

    pair var_p = variables;
    pair val_p = values;
    while (var_p) {
        assert(is_pair(var_p));
        assert(val_p);
        value var = car_of(var_p);
        value val = car_of(val_p);
        assert(var && is_symbol(var));
        table_set(t, var, val);
        var_p = cdr_of(var_p);
        val_p = cdr_of(val_p);
    }
    return ef;
}

void init_env(heap h, heap init)
{
    env_heap = h;
}
