#include <runtime.h>

static heap procedureheap;

void init_procedures(heap h, heap init)
{
    procedureheap = h;
}
