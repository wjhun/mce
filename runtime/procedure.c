#include <runtime.h>

/* Why was I convinced we needed a procedure type? */

static heap procedureheap;

procedure primitive_procedure(value (*fn)(value args))
{
    procedure p = allocate(procedureheap, sizeof(*p));
    if (p == INVALID_ADDRESS)
        return p;
    p->params = 0;
    p->fn = fn;
    p->env = 0;
    return p;
}

procedure compound_procedure(value params, value body, env_frame env)
{
    procedure p = allocate(procedureheap, sizeof(*p));
    if (p == INVALID_ADDRESS)
        return p;
    p->params = params;
    p->body = body;
    p->env = env;
    return p;
}

void init_procedures(heap h, heap init)
{
    procedureheap = h;
}
