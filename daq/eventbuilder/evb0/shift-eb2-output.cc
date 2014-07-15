#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include "b2eb_format.h"
#include "onsen_tcp.h"

class combined_event_t {

  uint32_t* m_hlt_buffer;
  uint32_t* m_onsen_buffer;
  SendHeader m_outer_header, m_hlt_header, m_onsen_header;

  SendTrailer m_outer_trailer, m_hlt_trailer, m_onsen_trailer;

  FILE* m_fp;

public:

  combined_event_t(FILE* fp) {
    if (fp)
      m_fp = fp;
    m_hlt_buffer   = new uint32_t[4 * 1024 * 1024];
    m_onsen_buffer = new uint32_t[4 * 1024 * 1024];
  };

  ~combined_event_t() {
    delete [] m_hlt_buffer;
    delete [] m_onsen_buffer;
  };

  int fetch(FILE* fin = NULL) {
    int ret;

    FILE* finput = fin ? fin : m_fp;

    ret = fread(&m_outer_header, sizeof(SendHeader), 1, finput);
    assert(ret == 1);

    assert(m_outer_header.nword_in_header * sizeof(uint32_t) == sizeof(SendHeader));

    ret = fread(&m_hlt_header, sizeof(m_hlt_header), 1, finput);
    assert(ret == 1);
    assert(m_hlt_header.nword_in_header * sizeof(uint32_t) == sizeof(m_hlt_header));

    ret = fread(m_hlt_buffer, m_hlt_header.nbyte_of_body(), 1, finput);
    assert(ret == 1);

    ret = fread(&m_hlt_trailer, sizeof(SendTrailer), 1, finput);
    assert(ret == 1);
    assert(m_hlt_trailer.magic == 0x7FFF0007);

    ret = fread(&m_onsen_header, sizeof(SendHeader), 1, finput);
    assert(ret == 1);
    assert(m_onsen_header.nword_in_header * sizeof(uint32_t) == sizeof(m_onsen_header));

    ret = fread(m_onsen_buffer, m_onsen_header.nbyte_of_body(), 1, finput);
    assert(ret == 1);

    assert(m_onsen_buffer[0] == 0xbebafeca);

    ret = fread(&m_onsen_trailer, sizeof(SendTrailer), 1, finput);
    assert(ret == 1);

    assert(m_onsen_trailer.magic == 0x7FFF0007);

    ret = fread(&m_outer_trailer, sizeof(SendTrailer), 1, finput);
    assert(ret == 1);

    assert(m_outer_trailer.magic == 0x7FFF0007);

    return m_outer_header.nword;
  };

  void fwrite_hlt_event(FILE* fout) const {
    int ret;
    ret = fwrite(&m_hlt_header, sizeof(m_hlt_header), 1, fout);
    assert(ret == 1);
    ret = fwrite(m_hlt_buffer, m_hlt_header.nbyte_of_body(), 1, fout);
    assert(ret == 1);
    ret = fwrite(&m_hlt_trailer, sizeof(m_hlt_trailer), 1, fout);
    assert(ret == 1);
  };

  void fwrite_onsen_event(FILE* fout) const {
    int ret;
    ret = fwrite(&m_onsen_header, sizeof(m_onsen_header), 1, fout);
    assert(ret == 1);
    ret = fwrite(m_onsen_buffer, m_onsen_header.nbyte_of_body(), 1, fout);
    assert(ret == 1);
    ret = fwrite(&m_onsen_trailer, sizeof(m_onsen_trailer), 1, fout);
    assert(ret == 1);
  };

  SendHeader outer_header() const {
    return m_outer_header;
  };

  SendHeader hlt_header() const {
    return m_hlt_header;
  };

  SendHeader onsen_header() const {
    return m_onsen_header;
  };

  SendTrailer outer_trailer() const {
    return m_outer_trailer;
  };

  SendTrailer hlt_trailer() const {
    return m_hlt_trailer;
  };

  SendTrailer onsen_trailer() const {
    return m_onsen_trailer;
  };


  uint32_t hlt_event_number() const {
    return m_hlt_header.event_number;
  };

  uint32_t onsen_trigger_number() const {
    const int nframe = ntohl(m_onsen_buffer[1]);
    sose_frame_t* sosep = (sose_frame_t*)&m_onsen_buffer[2 + nframe];
    assert(sosep->magic() == 0x58);
    return sosep->trigger_number();
  };
};

int
main(int argc, char** argv)
{
  int ret;

  int ch;

  int HLT_skip = 0;
  int ONSEN_skip = 0;

  FILE* HLT_fp;
  FILE* ONSEN_fp;

  while (-1 != (ch = getopt(argc, argv, "H:O:I:"))) {
    switch (ch) {
      case 'H':
        HLT_skip = strtol(optarg, 0, 0);
        break;
      case 'O':
        ONSEN_skip = strtol(optarg, 0, 0);
        break;
      case 'I':
        HLT_fp = fopen(optarg, "r");
        ONSEN_fp = fopen(optarg, "r");
    }
  }

  combined_event_t hlt(HLT_fp), onsen(ONSEN_fp);

  for (int i = 0; i < HLT_skip; i++) {
    hlt.fetch();
  }

  for (int i = 0; i < ONSEN_skip; i++) {
    onsen.fetch();
  }

  while (1) {

    hlt.fetch();
    onsen.fetch();

    SendHeader outer_header = hlt.outer_header();
    SendTrailer outer_trailer = hlt.outer_trailer();

    uint32_t i = outer_header.event_number;
    uint32_t h = hlt.hlt_event_number();
    uint32_t o = onsen.onsen_trigger_number();

    fprintf(stderr, "%d %d %d\n", i, h, o);

    outer_header.nword = hlt.hlt_header().nword + onsen.onsen_header().nword +
                         (sizeof(SendHeader) + sizeof(SendTrailer)) / sizeof(uint32_t);

    fwrite(&outer_header, sizeof(outer_header), 1, stdout);

    hlt.fwrite_hlt_event(stdout);
    onsen.fwrite_onsen_event(stdout);

    fwrite(&outer_trailer, sizeof(outer_trailer), 1, stdout);
  }
}
