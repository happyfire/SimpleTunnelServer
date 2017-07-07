#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include <stdio.h>

int g_verbose;

void* safe_malloc(size_t size);
void safe_free(void *buf);

#define LOG(format, ...) \
do{ \
    fprintf(stdout, "[TSLOG] "format"\n", ##__VA_ARGS__); \
}while(0)

#define LOGV(v, format, ...) \
do{ \
  if(g_verbose>=v){ \
    fprintf(stdout, "[TSLOG%d] "format" (%s:%d, %s)\n", v, ##__VA_ARGS__, __FILE__, __LINE__ , __FUNCTION__); \
  } \
}while(0)

#define LOGE(format, ...) \
do{ \
  fprintf(stderr, "[TSERR] "format" (%s:%d, %s)\n", ##__VA_ARGS__, __FILE__, __LINE__ , __FUNCTION__); \
}while(0)

#endif // _UTILS_H

