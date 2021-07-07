/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* util.h */


#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <errno.h>

#define ERROR(func) { fprintf(stderr, "[ERROR] %s:%d: "#func"(): %s\n", __FILE__, __LINE__, strerror(errno));}

extern void dump_binary(FILE* fp, const void* ptr, const size_t size);


#endif /* UTIL_H */

