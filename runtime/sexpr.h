typedef struct pair {
    value car;
    value cdr;
} *pair;

pair cons(value car, value cdr);

static inline boolean is_pair(value v)
{
    return v && tagof(v) == tag_pair;
}

parser sexp_parser(value_status_handler complete);

void init_sexprs(heap h, heap init);
