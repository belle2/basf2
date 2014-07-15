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
  int part = 0;
  int ch;
  int verbose = 0;
  int max_event = 0;

  while (-1 != (ch = getopt(argc, argv, "c:p:hv"))) {
    switch (ch) {
      case 'c':
        max_event = strtol(optarg, 0, 0);
        break;
      case 'v':
        verbose ++;
        break;
      case 'p':
        part = strtol(optarg, 0, 0);
        break;
      case 'h':
        fprintf(stderr, "-c #: how many events will be extracted\n-p #: specify the part number to extract. default = 0 (first)\n-v : increase verbose level\n");
        exit(1);
    }
  }

  argc -= optind;
  argv += optind;

  if (argc > 0) {
    fprintf(stderr, "open %s\n", argv[0]);
    in = fopen(argv[0], "r");
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

    if (verbose)
      fprintf(stderr, "nwh = %d\n", outer_header.nword_in_header);

    assert(sizeof(uint32_t) * outer_header.nword_in_header == sizeof(outer_header));

    /* Level1 inner header */

    for (int j = 0, remain = outer_header.nword_of_body(); remain > 0; j++) {

      if (verbose || event_number == 0)
        fprintf(stderr, "%d.%d\n", event_number, j);

      ret = fread(&block, sizeof(uint32_t) * 2, 1, in);
      assert(ret == 1);

      remain -= block.nword;

      if (verbose > 1)
        fprintf(stderr, "nwh = 0x%08x, remain = %d\n", block.nword_in_header, remain);


      switch (block.nword_in_header) {
        case 6:
          if (verbose)
            fprintf(stderr, "SendHeader or old FTSW (6words)\n");
          break;
        case 8:
          if (verbose)
            fprintf(stderr, "new FTSW (8words)\n");
          break;
        case 0x14:
          if (verbose)
            fprintf(stderr, "COPPER (20words)\n");
          break;
        default:
          assert("bad nword_in_header");
      }

      ret = fread(block.following, sizeof(uint32_t) * (block.nword - 2), 1, in);
      assert(ret == 1);

      if (j == part) {
        if (block.nword_in_header == 0x14) {
          /* COPPER */
          SendHeader dummy_header = outer_header;
          dummy_header.nboard(1);

          dummy_header.nword = block.nword + (sizeof(SendHeader) + sizeof(SendTrailer)) / sizeof(uint32_t);

          ret = fwrite(&dummy_header, sizeof(dummy_header), 1, stdout);
          assert(ret == 1);

          ret = fwrite(&block, sizeof(uint32_t) * block.nword, 1, stdout);
          assert(ret == 1);

          SendTrailer dummy_trailer;
          dummy_trailer.magic = 0x7FFF0007;
          ret = fwrite(&dummy_trailer, sizeof(SendTrailer), 1, stdout);
          assert(ret == 1);
        } else {
          ret = fwrite(&block, sizeof(uint32_t) * block.nword, 1, stdout);
          assert(ret == 1);
        }
      }
    }

    assert(sizeof(outer_trailer) == 8);

    ret = fread(&outer_trailer, sizeof(outer_trailer), 1, in);
    assert(ret == 1);
    assert((outer_trailer.magic & 0xFFFFFFF0) == 0x7FFF0000);

    if (max_event && max_event < event_number)
      break;
  }

}
