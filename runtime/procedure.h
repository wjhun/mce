#pragma once

typedef struct procedure {
    vector params;
    value body;
    env_frame env;
} *procedure;

// procedure-environment

// make-procedure

void init_procedures(heap h, heap init);
