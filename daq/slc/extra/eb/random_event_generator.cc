#include "b2eb_format.h"
#include "raw_copper_header_and_trailer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int nevent = 10;
int minsize = 4;
int maxsize = 16;

int quiet_flag = 0;

#define NBLOCK 1

#define WORDSIZE  (sizeof(uint32_t))

const char help_message[] =
  "h: help\n"
  "q: quiet\n"
  "n: number of events (default: 10)\n"
  "s: minimum number of words (default: 4)\n"
  "S: maximum number of words (default: 16)\n";

int
main(int argc, char** argv)
{
  int ch;
  while (-1 != (ch = getopt(argc, argv, "qhn:s:S:"))) {
    switch (ch) {
      case 'q':
        quiet_flag = 1;
        break;
      case 'h':
        fputs(help_message, stderr);
        exit(1);
        break;
      case 'n':
        nevent = strtol(optarg, 0, 0);
        break;
      case 's':
        minsize = strtol(optarg, 0, 0);
        break;
      case 'S':
        maxsize = strtol(optarg, 0, 0);
        break;
    }
  }

  struct SendHeader s_h;
  struct SendTrailer s_t;

  struct {
    struct RawCopperHeader_fixed_part fixed;
    struct RawCopperHeader_node node[NBLOCK];
    struct RawCopperHeader_end_of_header end;
  } c_h ;

  struct RawCopperTrailer c_t;

  char body[maxsize * WORDSIZE];

  for (int i = 0; i < nevent; i++) {

    int nword = minsize + (random() % (maxsize - minsize));
    int nbyte = nword * WORDSIZE;

    if (!quiet_flag) {
      fprintf(stderr, "%d %d\n", i, nword);
    }
    for (int j = 0; j < nbyte; j++) {
      body[j] = (i % 0xFF);
    }

    c_h.fixed.nword = (sizeof(c_h) + nbyte + sizeof(c_t)) / WORDSIZE;
    c_h.fixed.nword_in_header = sizeof(c_h) / WORDSIZE;
    c_h.fixed.number_of_node = NBLOCK;
    c_h.fixed.magic = 0x7FFF0004;
    c_h.end.magic = 0x7FFF0005;
    c_t.magic = 0x7FFF0006;

    c_h.fixed.experiment_number = -1;
    c_h.fixed.event_number = i;

    c_h.node[0].node_id = 1;
    c_h.node[0].event_number = i;

    s_h.nword = sizeof(s_h) / WORDSIZE + c_h.fixed.nword + sizeof(s_t) / WORDSIZE;
    s_h.nword_in_header = sizeof(s_h) / WORDSIZE;
    s_h.nboard(1);
    s_h.nevent(1);
    s_h.event_number = i;
    s_t.magic = 0x7FFF0007;
    fwrite(&s_h, sizeof(s_h), 1, stdout);
    fwrite(&c_h, sizeof(c_h), 1, stdout);
    fwrite(body, nword * WORDSIZE, 1, stdout);
    fwrite(&c_t, sizeof(c_t), 1, stdout);
    fwrite(&s_t, sizeof(s_t), 1, stdout);
  }

}
