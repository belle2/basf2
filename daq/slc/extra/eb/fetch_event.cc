#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "b2eb_format.h"
#include "strferror.h"

int verbose = 0;
int output = 0;

int
main(int argc, char** argv)
{
  uint32_t nword;

  int ch;
  while (-1 != (ch = getopt(argc, argv, "Ov"))) {
    switch (ch) {
      case 'O':
        output = 1;
        break;
      case 'v':
        verbose++;
        break;
    }
  }

  for (int e = 0; ; e++) {
    int ret;
    uint32_t buffer[1 * 1024 * 1024];

    if (feof(stdin)) {
      break;
    }
    /* read header */
    ret = fread(&buffer[0], sizeof(buffer[0]), 1, stdin);
    if (ret != 1) {
      if (0 != ferror(stdin)) {
        fprintf(stderr, "fread error: %s\n", strferror(stdin, "r"));
        abort();
      }
      break;
    }

    nword = buffer[0];
    if (verbose) {
      fprintf(stderr, "event#=%d nword=%d\n", e, nword);
    }

    /* read body */
    ret = fread(&buffer[1], sizeof(buffer[0]), nword - 1, stdin);
    if (ret != nword - 1) {
      int j;
      fprintf(stderr, "nevent = %d, request to fread %d, but returned %d\n",
              e, nword - 1, ret);
      for (j = 0; j < ret + 1; j++) {
        if (j % 8 == 0)
          fprintf(stderr, "%4d = ", j);
        fprintf(stderr, " %08x", buffer[j]);
        if (j % 8 == 7)
          fputc('\n', stderr);
      }
      if (j % 8 != 0)
        fputc('\n', stderr);
    }
    assert(ret == nword - 1);

    uint32_t e2 = buffer[4];

    if (verbose > 1) {
      fprintf(stderr, "event_number %d(from file head)/%d(data)\n", e, e2);
    }

    if (verbose > 2) {
      for (int i = 0; i < nword; i++) {
        fprintf(stderr, "%d %08x\n", i, buffer[i]);
      }
    }

    if (buffer[nword - 2] == 'TTD ' || buffer[nword - 2] == 'TLU ') {
      assert(buffer[nword - 1] == 0x7FFF0000);
    } else {
      assert(buffer[nword - 1] == 0x7FFF0007);
    }

    if (verbose > 1) {
      fprintf(stderr, "-2 %08x\n", buffer[nword - 2]);
      fprintf(stderr, "-1 %08x\n", buffer[nword - 1]);
    }

    if (output)
      fwrite(buffer, 4 * buffer[0], 1, stdout);
  }
  exit(0);
}
