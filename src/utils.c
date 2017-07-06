#include "utils.h"

void * safe_malloc(size_t size)
{
    void *buf = malloc(size);
    if (buf == NULL)
        exit(EXIT_FAILURE);
    return buf;
}

void safe_free(void *buf)
{
    if(buf != NULL) {
        free(buf);
        buf = NULL;
    }
}

