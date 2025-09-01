typedef struct pair {
    value car;
    value cdr;
} *pair;

pair cons(value car, value cdr);

static inline boolean is_pair(value v)
{
    return v && tagof(v) == tag_pair;
}

static inline value car_of(pair p)
{
    assert(is_pair(p));
    return p->car;
}

static inline value cdr_of(pair p)
{
    assert(is_pair(p));
    return p->cdr;
}

static inline value cadr_of(pair p)
{
    assert(is_pair(p));
    value cdr = cdr_of(p);
    assert(is_pair(cdr));
    return car_of(cdr);
}

static inline value caadr_of(pair p)
{
    value cadr = cadr_of(p);
    assert(is_pair(cadr));
    return car_of(cadr);
}

static inline value caddr_of(pair p)
{
    assert(is_pair(p));
    return cadr_of(cdr_of(p));
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
