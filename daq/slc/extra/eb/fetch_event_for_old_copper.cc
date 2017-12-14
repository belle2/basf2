#include "b2eb_format.h"
#include "raw_copper_header_and_trailer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX_EVENT_SIZE  (10 * 1024 * 1024)
#define MAXNODE   16

#define WORDSIZE  (sizeof(uint32_t))

void* fetch_event(FILE* fp, char* p_orig, int bufsize)
{
  char* p = p_orig;
  int ret;
  struct SendHeader s_h;
  struct SendTrailer s_t;

  struct RawCopperHeader_fixed_part c_h_f;
  struct RawCopperHeader_node c_h_n[MAXNODE];
  struct RawCopperHeader_end_of_header c_h_e;

  struct RawCopperTrailer c_t;

  assert(bufsize > sizeof(s_h));

  ret = fread(&s_h, sizeof(s_h), 1, fp);
  if (ret == 0 && feof(fp)) {
    return 0;
  }
  assert(ret == 1);
  assert(bufsize > s_h.nword * WORDSIZE);
  assert(sizeof(s_h) / 4 == s_h.nword_in_header);
  memcpy(p, &s_h, sizeof(s_h));
  p += sizeof(s_h);

  ret = fread(&c_h_f, sizeof(c_h_f), 1, fp);
  assert(ret == 1);
  assert(bufsize - sizeof(s_h)*WORDSIZE > c_h_f.nword * WORDSIZE);
  fprintf(stderr, "%x\n", c_h_f.magic);
  assert((c_h_f.magic & 0xFFFF0000) == 0x7FFF0000);
  assert(sizeof(c_h_f) + sizeof(c_h_e) + c_h_f.number_of_node * sizeof(c_h_n[0]) ==
         c_h_f.nword_in_header * WORDSIZE);
  memcpy(p, &c_h_f, sizeof(c_h_f));
  p += sizeof(c_h_f);

  for (int i = 0; i < c_h_f.number_of_node; i++) {
    struct RawCopperHeader_node& n = c_h_n[i];
    ret = fread(&n, sizeof(n), 1, fp);
    assert(ret == 1);
    memcpy(p, &n, sizeof(n));
    p += sizeof(n);
  }

  ret = fread(&c_h_e, sizeof(c_h_e), 1, fp);
  assert(ret == 1);
  assert(c_h_e.magic == 0x7FFF0005);
  memcpy(p, &c_h_e, sizeof(c_h_e));
  p += sizeof(c_h_e);

  ret = fread(p, (s_h.nword - s_h.nword_in_header - c_h_f.nword_in_header) * WORDSIZE, 1, fp);
  assert(ret == 1);

  uint32_t* lp = (uint32_t*)p_orig;
  assert(lp[s_h.nword - 1] == 0x7FFF0007);

  return p_orig;
}

main()
{
  int i = 0;
  static char p [MAX_EVENT_SIZE];

  while (p == fetch_event(stdin, p, MAX_EVENT_SIZE)) {
    fprintf(stderr, "%d\n", i++);
  }
}
