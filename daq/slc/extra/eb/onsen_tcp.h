#ifndef _ONSEN_TCP_H_
#define _ONSEN_TCP_H_

#include "neighbor.h"

#include <arpa/inet.h>
#include "b2eb_format.h"

const int max_onsen_event_size = 8 * 1024 * 1024;

const uint32_t onsen_magic = htonl(0xCAFEBABE);

#define MAX_PXD_FRAMES  256

class trig_frame_t {
public:
  uint16_t m_start; /* should be htonl(0x7000) */
  uint16_t m_nazo; /* should be htonl(0x7000) */
  uint32_t m_hlt;/* run_subrun; */
  uint32_t m_trigger;
  uint32_t m_exp_run;/* run_subrun; */

  uint32_t start() const
  {
    return ntohs(m_start);
  };

  uint32_t trigger_number() const
  {
    return ntohl(m_trigger);
  };

  uint32_t exp_run() const
  {
    return ntohl(m_exp_run);
  };
};

class evt_start_frame_t {
public:
  uint16_t start;
  uint16_t trigger_lo;
  uint16_t trigger_hi;
  uint16_t time_tag_lo;
  uint16_t time_tag_hi;
  uint32_t checksum;

  uint32_t trigger_number() const
  {
    return (ntohs(trigger_hi) << 16) + ntohs(trigger_lo);
  };

  uint32_t time_tag() const
  {
    return (ntohs(time_tag_hi) << 16) + ntohs(time_tag_lo);
  };
};

class sose_frame_t {
  uint16_t start; /* should be (HSB)X101-1XXX(LSB) */
  uint16_t trigger_lo;
  uint16_t trigger_hi;
  uint16_t time_tag_lo_and_type;
  uint16_t time_tag_mid;
  uint16_t time_tag_hi;
  uint16_t frame_counter;
  uint32_t crc32;
public:

  unsigned char magic() const
  {
    return *(unsigned char*)&start & 0x78;
  };

  uint32_t trigger_number() const
  {
    return (ntohs(trigger_hi) << 16) + ntohs(trigger_lo);
  };

  uint64_t time_tag() const
  {
    uint64_t retval = ((uint64_t)ntohs(time_tag_hi) << 32)
                      | ((uint64_t)ntohs(time_tag_mid) << 16)
                      | (uint64_t)ntohs(time_tag_lo_and_type);
    return (retval >> 4);
  };

  uint32_t trigger_type() const
  {
    return (ntohs(time_tag_lo_and_type) & 0xF);
  };
};

class onsen_tcp : public neighbor {
protected:
  SendHeader m_header;
  SendTrailer m_trailer;
  uint32_t m_magic;
  uint32_t m_framenr;
  uint32_t m_length_of_frame[MAX_PXD_FRAMES];
  uint8_t m_body[max_onsen_event_size];
public:

  onsen_tcp()
  {
    m_framenr = 0;
    m_header.nevent_nboard = 0x00010001;
    m_trailer.magic = 0x7FFF0007;
  };

  uint32_t framenr() const
  {
    return ntohl(m_framenr);
  };

  int nbyte_of_frames() const
  {
    const int nframe = framenr();

    int nbyte = 0;
    for (int i = 0; i < nframe; i++) {
      nbyte += ((ntohl(m_length_of_frame[i]) + 3) & ~3);
    }

    return nbyte;
  };

  int nbyte_of_body() const
  {
    const int nframe = framenr();

    return sizeof(m_magic)
           + sizeof(m_framenr)
           + nframe * sizeof(m_length_of_frame[0])
           + nbyte_of_frames()
           ;
  };

  int nbyte() const
  {
    return nbyte_of_body() + sizeof(m_header) + sizeof(m_trailer);
  };

  int nword() const
  {
    return nbyte() / 4;
  };

  int nword_of_body() const
  {
    return nbyte_of_body() / 4;
  };

  int magic() const
  {
    return ntohl(m_magic);
  };

  int fetch_event()
  {
    int ret;

    uint32_t dummy[100];


#if 1
    int nread = 0;
    do {
      ret = fread(&m_magic, sizeof(m_magic), 1, m_fp);
      if (ret != 1) {
        log("failed to read MAGIC(0x%08x) for ONSEN: %s\n", onsen_magic, strferror());
        exit(1);
      }
      nread ++;
    } while (m_magic != onsen_magic);

    if (nread > 1) {
      log("%d words are skipped to find 0x%08x\n", nread - 1, onsen_magic);
    }

    ret = fread(&m_framenr, sizeof(m_framenr), 1, m_fp);
    if (ret != 1) {
      log("failed to read FRAMENR for ONSEN: %s\n", strferror());
      exit(1);
    }

#else

    ret = fread(&m_magic, sizeof(m_magic) + sizeof(m_framenr), 1, m_fp);
    if (ret != 1) {
      log("failed to read MAGIC+FRAMENR for ONSEN: %s\n", strferror());
      exit(1);
    }

    if (m_magic != onsen_magic) {
      log("onsen_magic %08x != %08x\n", ntohl(m_magic), ntohl(onsen_magic));
      dump_event();
      exit(1);
    }
#endif

    const int nframe = framenr();

    if (nframe > MAX_PXD_FRAMES) {
      log("framenr (%d) > %d\n", nframe, MAX_PXD_FRAMES);
      exit(1);
    }

    ret = fread(m_length_of_frame,
                sizeof(m_length_of_frame[0]) * nframe, 1, m_fp);

    if (ret != 1) {
      log("failed to read length of frame for %d frames\n", nframe);
      exit(1);
    }

    int expected_size = nbyte_of_frames();

    ret = fread(m_body, expected_size, 1, m_fp);
    if (ret != 1) {
      log("failed to read %d frames in %d bytes from ONSEN: %s\n", nframe, expected_size, strferror());
      exit(1);
    }

    // assert(m_body[0] & 0x78 == 0x58);

    m_header.nword = nword();

    return nbyte();
  };

  SendHeader header() const
  {
    return m_header;
  };

  SendTrailer trailer() const
  {
    return m_trailer;
  };


  int send_event(FILE* dest) const
  {
    int ret;

    ret = fwrite(&m_header, sizeof(m_header), 1, dest);
    if (ret != 1) {
      log("failed to write SendHeader: %s\n", strferror(dest, "w"));
      exit(1);
    }

    ret = fwrite(&m_magic, sizeof(m_magic), 1, dest);
    if (ret != 1) {
      log("failed to write MAGIC: %s\n", strferror(dest, "w"));
      exit(1);
    }

    ret = fwrite(&m_framenr, sizeof(m_framenr), 1, dest);
    if (ret != 1) {
      log("failed to write framenr: %s\n", strferror(dest, "w"));
      exit(1);
    }

    const int nframe = framenr();

    ret = fwrite(m_length_of_frame,
                 nframe * sizeof(m_length_of_frame[0]), 1, dest);

    if (ret != 1) {
      log("failed to write length_of_frame: %s\n", strferror(dest, "w"));
      exit(1);
    }

    ret = fwrite(m_body, nbyte_of_frames(), 1, dest);
    if (ret != 1) {
      log("failed to write ONSEN frames: %s\n", strferror(dest, "w"));
      exit(1);
    }

    ret = fwrite(&m_trailer, sizeof(m_trailer), 1, dest);
    if (ret != 1) {
      log("failed to write SendTrailer: %s\n", strferror(dest, "w"));
      exit(1);
    }

    return nbyte();
  };

  uint32_t event_number() const
  {
#if 0
    sose_frame_t* sose = (sose_frame_t*)m_body;
    return sose->trigger_number();
#else
    trig_frame_t* trig = (trig_frame_t*)m_body;
    return trig->trigger_number();
#endif
  };

#if 0
  uint64_t time_tag() const
  {
    sose_frame_t* sose = (sose_frame_t*)m_body;
    return sose->time_tag();
  };
#else
  uint32_t exp_run() const
  {
    trig_frame_t* trig = (trig_frame_t*)m_body;
    return trig->exp_run();
  };

  uint64_t event_id64() const
  {
    return (((uint64_t)exp_run() << 32) | event_number());
  };
#endif

  void dump_event()
  {
    uint16_t* p = (uint16_t*)m_body;

    log("%8d 0x%08x\n", 0, ntohl(m_magic));
    log("%8d 0x%08x\n", 1, ntohl(m_framenr));

    const int nframe = framenr();

    log("event_number = %ld\n", event_number());
#if 0
    log("time_tag = %lld\n", time_tag());
#else
    log("exp_run = %08x\n", exp_run());
#endif

    for (int i = 0; i < nframe; i++) {
      uint32_t v = ntohl(m_length_of_frame[i]);
      log("%8d 0x%08x %d\n", 2 + i, v, v);
    }

    for (int i = 0; i < nbyte_of_frames() / 2; i++) {
      uint16_t v = ntohs(p[i]);
      log("%8d 0x%04x %d\n", (2 + nframe + i) / 2, v, v);
    }
  };

  uint64_t skip_to(uint64_t expected)
  {
    while (event_id64() < expected) {
      log("skip bogus event from %s:%s exp_run:0x%08lx event:0x%08lx < expected 0x%016llx\n",
          m_remote_host,
          m_remote_port,
          exp_run(),
          event_number(),
          expected);
      fetch_event();
    };
  };

};
#endif
