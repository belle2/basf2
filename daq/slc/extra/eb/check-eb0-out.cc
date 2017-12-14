#include <stdio.h>
#include <assert.h>
#include "ftsw_tcp.h"

struct {
  uint32_t nword;
  uint32_t nword_in_header;
  uint32_t following[4 * 1024 * 1024];
} block;

int
main(int argc, char** argv)
{
  FILE* in = stdin;

  if (argc > 1) {
    fprintf(stderr, "open %s\n", argv[1]);
    in = fopen(argv[1], "r");
    if (in == NULL) {
      perror("open:");
      exit(1);
    }
  }

  for (int event_number = 0 ;; event_number++) {
    int ret;

    fprintf(stderr, "%d\n", event_number);

    /* read outer header */

    SendHeader  outer_header;
    SendTrailer outer_trailer;

    ret = fread(&outer_header, sizeof(outer_header), 1, in);
    assert(ret == 1);

    fprintf(stderr, "nwh = %d\n", outer_header.nword_in_header);

    assert(sizeof(uint32_t) * outer_header.nword_in_header == sizeof(outer_header));

    /* Level1 inner header */

    for (int j = 0, remain = outer_header.nword_of_body(); remain > 0; j++) {

      fprintf(stderr, "%d.%d\n", event_number, j);

      ret = fread(&block, sizeof(uint32_t) * 2, 1, in);
      assert(ret == 1);

      remain -= block.nword;

      fprintf(stderr, "nwh = 0x%08x, remain = %d\n", block.nword_in_header, remain);


      switch (block.nword_in_header) {
        case 6:
          fprintf(stderr, "SendHeader or old FTSW (6words)\n");
          break;
        case 8:
          fprintf(stderr, "new FTSW (8words)\n");
          break;
        case 0x14:
          fprintf(stderr, "COPPER (20words)\n");
          break;
        default:
          assert("bad nword_in_header");
      }

      ret = fread(block.following, sizeof(uint32_t) * (block.nword - 2), 1, in);
      assert(ret == 1);

      if (block.nword_in_header == 0x14) {
        for (int k = 0; k < 8; k++) {
          printf("%2d %08x\n", k + 2, block.following[k]);
        }
      }
    }

    assert(sizeof(outer_trailer) == 8);

    ret = fread(&outer_trailer, sizeof(outer_trailer), 1, in);
    assert(ret == 1);
    assert((outer_trailer.magic & 0xFFFFFFF0) == 0x7FFF0000);
  }

}
