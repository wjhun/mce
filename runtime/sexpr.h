typedef struct pair {
    value car;
    value cdr;
} *pair;

pair cons(value car, value cdr);

static inline boolean is_pair(value v)
{
    return v && tagof(v) == tag_pair;
}

static inline int pair_list_length(pair p)
{
    int n = 1;
    while (p->cdr) {
        if (!is_pair(p->cdr))
            return -1;
        p = p->cdr;
        n++;
    }
    return n;
}

parser sexp_parser(value_status_handler complete);

void init_sexprs(heap h, heap init);
