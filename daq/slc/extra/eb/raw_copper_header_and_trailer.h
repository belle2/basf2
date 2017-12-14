#ifndef _RAW_COPPER_HEADER_AND_TRAILER_H_
#define _RAW_COPPER_HEADER_AND_TRAILER_H_

#include <stdint.h>

struct RawCopperHeader_fixed_part {
  uint32_t nword; /* number of total words including header and trailer */
  uint32_t nword_in_header; /* number of words in this header */
  uint32_t experiment_number; /* number of words in this header */
  uint32_t event_number;
  uint32_t reserved_1;
  uint32_t subsystem_id;
  uint32_t type_of_data;
  uint32_t truncation_mask;
  uint32_t number_of_b2link_block;
  uint32_t offset_to_b2link_block[4];
  uint32_t reserved_2;
  uint32_t reserved_3;
  uint32_t magic; /* 0x7FFF0004 */
  uint32_t number_of_node;
};

struct RawCopperHeader_node {
  uint32_t node_id;
  uint32_t event_number;
};

struct RawCopperHeader_end_of_header {
  uint32_t magic; /* 0x7FFF0005 */
};

struct RawCopperTrailer {
  uint32_t reserved;
  uint32_t magic; /* 0x7FFF0006 */
};
#endif
