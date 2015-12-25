#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "b2eb_format.h"
#include "onsen_tcp.h"

uint32_t hlt_buffer [4 * 1024 * 1024];
uint32_t onsen_buffer [4 * 1024 * 1024];
SendHeader outer_header, onsen_header;

struct abstract_header_t {
  uint32_t nword;
  uint32_t nword_in_header;
  uint32_t data[256];
} hlt_header;

SendTrailer outer_trailer, onsen_trailer;

void
dump_abstract_header(const char* name, void* v)
{
  uint32_t* p = (uint32_t*)v;
  int n = p[1];
  for (int i = 0; i < n; i++) {
    printf("%s[%2d] = 0x%08x | %d\n", name, i, p[i], p[i]);
  }
  putchar('\n');
}

int
main(int argc, char** argv)
{
  int ret;

  for (int evtno = 0; ; evtno++) {
    ret = fread(&outer_header, sizeof(SendHeader), 1, stdin);
    assert(ret == 1);

    /* dump_abstract_header("EB2", &outer_header); */

    assert(outer_header.nword_in_header * sizeof(uint32_t) == sizeof(SendHeader));

    ret = fread(&hlt_header, sizeof(uint32_t) * 2, 1, stdin);
    assert(ret == 1);

    ret = fread(hlt_header.data, sizeof(uint32_t) * (hlt_header.nword_in_header - 2), 1, stdin);
    assert(ret == 1);

#if 0
    dump_abstract_header("HLT", &hlt_header);
#endif

    ret = fread(hlt_buffer, sizeof(uint32_t) * (hlt_header.nword - hlt_header.nword_in_header), 1, stdin);
    assert(ret == 1);

#if 0
    for (int i = 0; i < 20; i++) {
      printf("%2d %08x | %d\n", i, hlt_buffer[i], hlt_buffer[i]);
    }
#endif

    ret = fread(&onsen_header, sizeof(SendHeader), 1, stdin);
    assert(ret == 1);

    /* dump_abstract_header("PXD", &onsen_header); */

    ret = fread(&onsen_buffer, onsen_header.nbyte_of_body(), 1, stdin);
    assert(ret == 1);

#if 0
    for (int i = 0; i < 20; i++) {
      printf("%08x\n", ntohl(onsen_buffer[i]));
    }
#endif

    assert(onsen_buffer[0] == 0xbebafeca);
    const int nframe = ntohl(onsen_buffer[1]);

    sose_frame_t* sosep = (sose_frame_t*)&onsen_buffer[2 + nframe];
    assert(sosep->magic() == 0x58);

    const uint32_t onsen_trigger_number = sosep->trigger_number();
    const uint64_t onsen_time_tag = sosep->time_tag();

    /* printf("%08lx %16llx\n", onsen_trigger_number, onsen_time_tag); */

    ret = fread(&onsen_trailer, sizeof(SendTrailer), 1, stdin);
    assert(ret == 1);

    assert(onsen_trailer.magic == 0x7FFF0007);

    ret = fread(&outer_trailer, sizeof(SendTrailer), 1, stdin);
    assert(ret == 1);

    assert(outer_trailer.magic == 0x7FFF0007);

    printf("%5d %08x %5d %5d\n", evtno, outer_header.exp_run, outer_header.event_number, onsen_trigger_number);
  }
}
