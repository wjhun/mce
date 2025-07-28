#include <runtime.h>

static heap env_heap;

// embed make-frame
// vars,vals may actually be vectors - handle either pair or vector?
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
    return ef;
}

void init_env(heap h, heap init)
{
    env_heap = h;
}
