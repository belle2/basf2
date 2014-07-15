#ifndef _FTSW_TCP_H_
#define _FTSW_TCP_H_
#include "neighbor.h"
#include "b2eb_format.h"

struct ftsw_header_t {
  uint32_t nword; /* number of total words including header */
  uint32_t nword_in_header; /* number of words of this header == 6 */
  uint32_t nevent_nboard; /* number of board in this block, mostly # of COPPER */
  uint32_t exp_run;
  uint32_t event_number;
  uint32_t reserved1;
  uint32_t node_id;
  uint32_t reserved2;
};

class ftsw_tcp : public neighbor {
public:
  static const int max_event_size = 1 * 1024 * 1024;

protected:
  ftsw_header_t m_header;
  char m_buffer[max_event_size]; /* store an event */
  const char* m_mode;

public:

  int is_ftsw() const {
    if (m_header.node_id == 'TTD ') {
      return 1;
    } else {
      return 0;
    }
  };

  int is_tlu() const {
    if (m_header.node_id == 'TLU ') {
      return 1;
    } else {
      return 0;
    }
  };

  int nword() const {
    return m_header.nword;
  };

  int nbyte() const {
    return nword() * 4;
  };

  int fetch_event() {
    int ret;
    ret = fread(&m_header, sizeof(m_header), 1, m_fp);
    if (ret == 0 && ferror(m_fp) == EPERM && m_cleared) {
      log("fetch header cancelled\n");
      return 0;
    }

    if (m_header.nword_in_header * 4 != sizeof(m_header)) {
      log("unexpected header size 0x%08x words from FTSW\n", m_header.nword_in_header);
      uint32_t* p = (uint32_t*)&m_header;
      for (int i = 0; i < 8; i++) {
        log("%d %08x\n", i, p[i]);
      }
      exit(0);
    }

    if (ret != 1) {
      log("failed to fetch header from %s: %s\n", m_remote_host, strferror());
      exit(1);
    }

    int body_and_trailer_size = nbyte() - sizeof(m_header);

    ret = fread(m_buffer, body_and_trailer_size, 1, m_fp);
    if (ret == 0 && ferror(m_fp) == EPERM && m_cleared) {
      log("fetch body cancelled\n");
      return 0;
    }

    if (ret != 1) {
      if (feof(m_fp)) {
        log("failed to fetch body (%d bytes) from %s: EOF\n",
            body_and_trailer_size,
            m_remote_host);
      } else {
        log("failed to fetch body (%d bytes) from %s: %s\n",
            body_and_trailer_size,
            m_remote_host, strferror());
      }
      exit(1);
    }


    return nbyte();
  };

  int send_event(FILE* dest) {
    int ret;


    ret = fwrite(&m_header, sizeof(m_header), 1, dest);
    if (ret != 1) {
      log("failed to write header: %s\n", strferror(dest, "w"));
      exit(1);
    }
    ret = fwrite(m_buffer, nbyte() - sizeof(m_header), 1, dest);
    if (ret != 1) {
      log("failed to write body: %s\n", strferror(dest, "w"));
      exit(1);
    }

    return nbyte();
  };

  ftsw_header_t header() {
    return m_header;
  };

  int nbyte_of_body() const {
    return nbyte() - sizeof(ftsw_header_t) - sizeof(SendTrailer);
  };

  int nword_of_body() const {
    return nword() - (sizeof(ftsw_header_t) + sizeof(SendTrailer)) / 4;
  };

  int send_body(FILE* dest) {
    int ret;
    ret = fwrite(m_buffer, nbyte_of_body(), 1, dest);
    if (ret != 1) {
      log("failed to write body :%s\n", strferror(dest, "w"));
    }
    return nbyte_of_body();
  };

  int send_to_basf(FILE* dest) {
    if (is_ftsw() || is_tlu()) {
      return send_event(dest);
    } else {
      return send_body(dest);
    }
  };

  SendTrailer trailer() {
    SendTrailer trailer;
    memcpy(&trailer, m_buffer + nbyte_of_body(), sizeof(trailer));
    return trailer;
  };

  uint32_t trailer_magic() {
    uint32_t* p = (uint32_t*)&m_header;
    return p[nword() - 1];
  };

  void dump_event() const {
    uint32_t* p = (uint32_t*)&m_header;
    log("===============\n");
    for (int i = 0; i < nword(); i++) {
      log("%d %08x\n", i, p[i]);
    }
    log("===============\n");
  };

  uint64_t event_id64() const {
    return ((uint64_t)m_header.exp_run << 32 | m_header.event_number);
  };

  uint64_t skip_to(uint64_t expected) {
    while (event_id64() < expected) {
      log("skip bogus event from %s:%s exp_run:0x%08lx event:0x%08lx < expected 0x%016llx\n",
          m_remote_host,
          m_remote_port,
          m_header.exp_run,
          m_header.event_number,
          expected);
      fetch_event();
    };
  };

  uint32_t utc() const {
    uint32_t* p32 = (uint32_t*)m_buffer;
    return p32[1];
  };

  uint64_t time_tag() const {
    uint64_t* p64 = (uint64_t*)m_buffer;
    return p64[0];
  };
};

#endif
