#pragma once

typedef struct procedure {
    value params;           /* primitive if null */
    union {
        value (*fn)(value args); /* primitive */
        value body;
    };
    env_frame env;
} *procedure;

// procedure-environment

// make-procedure

static inline boolean is_procedure(value v) {
    return v && tagof(v) == tag_procedure;
}

static inline boolean procedure_is_primitive(procedure p) {
    return !p->params;
}

procedure primitive_procedure(value (*fn)(value args));

procedure compound_procedure(value params, value body, env_frame env);

void init_procedures(heap h, heap init);
