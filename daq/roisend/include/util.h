/* util.h */


#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <errno.h>

#define ERROR(func) { fprintf(stderr, "[ERROR] %s:%d: "#func"(): %s\n", __FILE__, __LINE__, strerror(errno));}

extern void dump_binary(FILE* fp, const void* ptr, const size_t size);


#endif /* UTIL_H */

