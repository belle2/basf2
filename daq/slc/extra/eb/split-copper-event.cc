#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

struct {
  uint32_t nword;
  uint32_t nword_in_header;
  uint32_t following[4 * 1024 * 1024];
} block;

int
main(int argc, char** argv)
{
  int ret;
  char* filename = "stdin";
  FILE* in = stdin;

  if (argc > 1) {
    filename = argv[1];
    in = fopen(filename, "r");
    if (in == NULL) {
      perror("fopen:");
      exit(1);
    }
    assert(in);
    fprintf(stderr, "open %s\n", filename);
  }

  for (int event_number = 0;; event_number++) {
    ret = fread(&block, sizeof(uint32_t) * 2, 1, in);
    assert(ret == 1);

    switch (block.nword_in_header) {
      case 6:
      case 8:
      case 0x14:
        break;
      default:
        assert("bad nword_in_header");
    }

    fread(block.following, sizeof(uint32_t) * (block.nword - 2), 1, in);
    assert(ret == 1);

    char dumpfilename[1024];

    snprintf(dumpfilename, sizeof(dumpfilename),
             "%s.%d", filename, event_number);

    assert(access(dumpfilename, F_OK) == -1);

    FILE* out = fopen(dumpfilename, "w");
    ret = fwrite(&block, sizeof(uint32_t) * block.nword, 1, out);
    assert(ret == 1);
    fclose(out);
  }
}
