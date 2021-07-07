/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/* util.c */

#include <stdio.h>


void
dump_binary(FILE* fp, const void* ptr, const size_t size)
{
  int i;
  const unsigned int* p = (const unsigned int*)ptr;
  const size_t _size = size / sizeof(unsigned int);


  for (i = 0; i < _size; i++) {
    fprintf(fp, "%08x ", p[i]);
    if (i % 8 == 7) fprintf(fp, "\n");
  }
  if (_size % 8 != 0) fprintf(fp, "\n");
}

