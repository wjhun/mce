#include <runtime.h>

static heap sexpheap;

static boolean char_is_whitespace(character in)
{
    return (runtime_strchr(" \n\r\t", in) != 0);
}

// XXX tuple_parser.c, move
typedef closure_type(completion, parser, void *);
#define PARSE_END 0

static boolean is_digit(character in)
{
    return (runtime_strchr("1234567890", in) != 0);
}

// XXX error path
closure_function(3, 1, parser, parse_number,
                 completion, next,
                 s64, value,
                 boolean, negative,
                 character, in)
{
    // no guard rails
    if (is_digit(in)) {
        int inc = in - '0';
        bound(value) = bound(value) * 10 + inc;
        return (parser)closure_self();
    }
    value v = value_from_s64(bound(value) * (bound(negative) ? -1 : 1));
    parser p = apply(bound(next), v);
    return p ? apply(p, in) : 0;
}

closure_function(3, 1, parser, parse_string,
                 completion, next, string, s, boolean, escaped,
                 character, in)
{
    if (!bound(escaped)) {
        if (in == '\\') {
            bound(escaped) = true;
            return (parser)closure_self();
        }
        if (in == '"') {
            return apply(bound(next), bound(s));
        }
        push_u8(bound(s), in);
    } else {
        if (in == '"' ||
            in == '\\') {
            push_u8(bound(s), in);
        }
        bound(escaped) = false;
    }
    return (parser)closure_self();
}

static boolean is_sym_delimiter(character in)
{
    return (runtime_strchr(" \n\r\t()", in) != 0);
}

// XXX error path
closure_function(2, 1, parser, parse_symbol,
                 completion, next, string, s,
                 character, in)
{
    // no guard rails
    if (!is_sym_delimiter(in)) {
        push_character(bound(s), in);
        return (parser)closure_self();
    }
    parser p = apply(bound(next), intern(bound(s)));
    deallocate_string(bound(s));
    bound(s) = 0;
    return p ? apply(p, in) : 0;
}

pair cons(value car, value cdr)
{
    pair p = allocate(sexpheap, sizeof(struct pair));
    // ref
    p->car = car;
    p->cdr = cdr;
    return p;
}

closure_function(3, 1, parser, eat_closing_paren,
                 completion, next, value, car, value, cdr,
                 character, in)
{
    if (char_is_whitespace(in) || (in == CHARACTER_INVALID))
        return (parser)closure_self();

    if (in != ')') {
        rprintf("err\n");
        // fix error handling
        apply(bound(next), 0);
        return 0;
    }

    pair p = cons(bound(car), bound(cdr));
    return apply(bound(next), p);
}

closure_function(2, 1, parser, cdr_complete,
                 completion, next, value, car,
                 void *, p)
{
    value cdr = (value)p;
    return (parser)closure(transient, eat_closing_paren, bound(next), bound(car), cdr);
}

/* no parsing at this point, just consing up the chain and passing to caller */
closure_function(2, 1, parser, cons_complete,
                 completion, next, value, car,
                 void *, p)
{
    value cdr = (value)p;
    assert(is_null(cdr) || tagof(cdr) == tag_pair);
    return apply(bound(next), cons(bound(car), cdr));
}

declare_closure_function(1, 1, parser, next_element,
                         completion, next,
                         void *, p);

declare_closure_function(1, 1, parser, parse_sexp,
                         completion, next,
                         character, in);

closure_function(1, 1, parser, parse_end_of_list_or_more,
                 completion, next,
                 character, in)
{
    if (char_is_whitespace(in) || (in == CHARACTER_INVALID))
        return (parser)closure_self();

    if (in == ')') {
        return apply(bound(next), 0); // XXX singleton
    }

    completion c_next = (completion)closure(transient, next_element, bound(next));
    parser p = (parser)closure(transient, parse_sexp, c_next);
    return apply(p, in);
}

define_closure_function(1, 1, parser, next_element,
                        completion, next,
                        void *, p)
{
    value car = (value)p;

    completion c = (completion)closure(transient, cons_complete, bound(next), car);
    return (parser)closure(transient, parse_end_of_list_or_more, c);
}

closure_function(2, 1, parser, pair_or_list,
                 completion, next, value, car,
                 character, in)
{
    if (char_is_whitespace(in) || (in == CHARACTER_INVALID))
        return (parser)closure_self();

    if (in == '.') {
        completion c = (completion)closure(transient, cdr_complete, bound(next), bound(car));
        return (parser)closure(transient, parse_sexp, c);
    } else if (in == ')') {
        pair p = cons(bound(car), 0); // find nil singleton
        return apply(bound(next), p);
    }

    // refactor with parse_end_of_list_or_more
    completion c_cons = (completion)closure(transient, cons_complete, bound(next), bound(car));
    completion c_next = (completion)closure(transient, next_element, c_cons);
    parser p = (parser)closure(transient, parse_sexp, c_next);
    return apply(p, in);
}

closure_function(1, 1, parser, car_complete,
                 completion, next,
                 void *, p)
{
    value car = (value)p;
    return (parser)closure(transient, pair_or_list, bound(next), car);
}

closure_function(1, 1, parser, quote_complete,
                 completion, next,
                 void *, p)
{
    value cdr = (value)p;
    value car = sym(quote);
    return apply(bound(next), cons(car, cons(cdr, 0)));
}

define_closure_function(1, 1, parser, parse_sexp,
                        completion, next,
                        character, in)
{
    if (char_is_whitespace(in) || (in == CHARACTER_INVALID))
        return (parser)closure_self();
    
    if (in == '(') {
        completion c = (completion)closure(transient, car_complete, bound(next));
        parser p = (parser)closure(transient, parse_sexp, c);
        return p;
    } else {
        // XXX signed int only
        // XXX incomplete, we need to allow for '-' and '+' identifiers...
        if (in == '-') {
            parser p = (parser)closure(transient, parse_number, bound(next), 0, true);
            return p;
        } else if (is_digit(in)) {
            parser p = (parser)closure(transient, parse_number, bound(next), 0, false);
            return apply(p, in);
        } else if (in == '\'' || in == '`') {
            completion c = (completion)closure(transient, quote_complete, bound(next));
            return (parser)closure(transient, parse_sexp, c);
        } else if (in == '\"') {
            return (parser)closure(transient, parse_string, bound(next), allocate_string(32), false);
        } else {
            /* default to symbol */
            parser p = (parser)closure(transient, parse_symbol, bound(next), allocate_string(32));
            return apply(p, in);
        }
    }
    apply(bound(next), 0);
    return (parser)closure_self();
}

/* this is a "completion" that consumes the produced sexp and invokes the upstream handler
   how can we do without this munging? */
closure_function(1, 1, parser, parse_sexp_bridge,
                 value_status_handler, next,
                 void *, p)
{
    value v = (value)p;
    apply(bound(next), v, STATUS_OK);
    closure_finish();
    return PARSE_END;
}

parser sexp_parser(value_status_handler complete)
{
    completion c = closure(transient, parse_sexp_bridge, complete);
    return (parser)closure(transient, parse_sexp, c);
}

void init_sexprs(heap h, heap init)
{
    sexpheap = h;
}

/* misc sexpr helpers */

static pair cons_list_arg(unsigned int remain, vlist *args)
{
    if (remain == 0)
        return 0;
    value v = varg(*args, value);
    return cons(v, cons_list_arg(remain - 1, args));
}

pair list_from_args(int num, ...)
{
    vlist a;
    vstart(a, num);
    pair l = cons_list_arg(num, &a);
    vend(a);
    return l;
}
