typedef struct pair {
    value car;
    value cdr;
} *pair;

pair cons(value car, value cdr);

static inline boolean is_null(value v)
{
    // XXX singleton
    return !v;
}

static inline boolean is_pair(value v)
{
    return !is_null(v) && tagof(v) == tag_pair;
}

static inline value car_of(pair p)
{
    assert(is_pair(p));
    return p->car;
}

static inline value caar_of(pair p)
{
    assert(is_pair(p));
    return car_of(p->car);;
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

static inline value cdadr_of(pair p)
{
    value cadr = cadr_of(p);
    assert(is_pair(cadr));
    return cdr_of(cadr);
}

static inline value cddr_of(pair p)
{
    value cdr = cdr_of(p);
    assert(is_pair(cdr));
    return cdr_of(cdr);
}

static inline value caadr_of(pair p)
{
    value cadr = cadr_of(p);
    assert(is_pair(cadr));
    return car_of(cadr);
}

static inline value caddr_of(pair p)
{
    value cddr = cddr_of(p);
    return car_of(cddr);
}

static inline value cdddr_of(pair p)
{
    value cddr = cddr_of(p);
    return cdr_of(cddr);
}

static inline value cadddr_of(pair p)
{
    value cdddr = cdddr_of(p);
    return car_of(cdddr);
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

pair list_from_args(int num, ...);

parser sexp_parser(value_status_handler complete);

void init_sexprs(heap h, heap init);
