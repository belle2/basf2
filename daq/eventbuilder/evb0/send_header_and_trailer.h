#ifndef _SEND_HEADER_AND_TRAILER_H_
#define _SEND_HEADER_AND_TRAILER_H_

#include <stdint.h>

struct SendHeader {
  uint32_t nword; /* number of total words including header */
  uint32_t nword_in_header; /* number of words of this header */
  uint32_t nevent; /* number of events in this block */
  uint32_t reserved;
};

struct SendTrailer {
  uint32_t reserved;
  uint32_t magic; /* should be 0x7FFF000X */
};
#endif
