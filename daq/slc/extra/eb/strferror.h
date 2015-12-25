#ifndef _STRFERROR_H_
#define _STRFERROR_H_

#include <stdio.h>
#include <errno.h>

const char*
strferror(FILE* fp, const char* mode)
{
  if (mode == NULL)
    mode = "r";

  if (feof(fp)) {
    switch (mode[0]) {
      case 'r':
        return "EOF for read";
        break;
      case 'w':
        return "EOF for write";
        break;
      default:
        return "EOF";
        break;
    }
  }

  if (ferror(fp)) {
    return strerror(errno);
  } else {
    return "no error from ferror";
  }
}

#endif
