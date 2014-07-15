#include <unistd.h>
#include "onsen_tcp.h"

onsen_tcp o;

int quiet = 0;
int verbose = 0;

int
main(int argc, char** argv)
{
  int nevent = 0;
  int ch;
  while (-1 != (ch = getopt(argc, argv, "vq"))) {
    switch (ch) {
      case 'v':
        verbose++;
        break;
      case 'q':
        quiet = 1;
        break;
    }
  }

  o.open(0, "r");

  for (int nevent = 0; ; nevent++) {
    o.fetch_event();
    fprintf(stderr, "reading event %d\n", nevent);
    if (verbose)
      o.dump_event();
    assert(o.header().nword == o.nword());

    if (!quiet)
      o.send_event(stdout);
  }
}
