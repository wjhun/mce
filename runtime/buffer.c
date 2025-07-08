#include <runtime.h>

buffer allocate_buffer(heap h, bytes s)
{
    buffer b = allocate(h, sizeof(struct buffer));
    if (b == INVALID_ADDRESS)
        return b;
    void *contents = allocate(h, s);
    if (contents == INVALID_ADDRESS) {
        deallocate(h, b, sizeof(struct buffer));
        return INVALID_ADDRESS;
    }
    init_buffer(b, s, false, h, contents);
    return b;
}

boolean buffer_append(buffer b,
                     const void *body,
                     bytes length)
{
    if (!buffer_extend(b, length))
        return false;
    return buffer_write(b, body, length);
}

int buffer_strstr(buffer b, const char *str) {
    int len = runtime_strlen(str);
    for (int i = 0; b->start + i + len <= b->end; i++) {
        if (!runtime_memcmp(buffer_ref(b, i), str, len))
            return i;
    }
    return -1;
}

void buffer_print(buffer b)
{
    console_write(buffer_ref(b, 0), buffer_length(b));
}
