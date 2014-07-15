#ifndef _B2EB_FORMAT_H_
#define _B2EB_FORMAT_H_

#include <stdint.h>

struct SendTrailer {
  uint32_t reserved;
  uint32_t magic; /* should be 0x7FFF000X */
};

class SendHeader {
public:
  uint32_t nword; /* number of total words including header */
  uint32_t nword_in_header; /* number of words of this header == 6 */
  uint32_t nevent_nboard; /* number of board in this block, mostly # of COPPER */
  uint32_t exp_run;
  uint32_t event_number;
  uint32_t node_id;

  SendHeader() :
    nword_in_header(sizeof(SendHeader) / 4),
    nword((sizeof(SendHeader) + sizeof(SendTrailer)) / 4),
    nevent_nboard(0),
    exp_run(0),
    event_number(0),
    node_id(0) { };

  int exp() const {
    return (exp_run >> 22) & 0x3FF;
  };

  int run() const {
    return (exp_run & 0x3FFFFF);
  };

  int exp(int eno) {
    exp_run = ((eno & 0x3FF) << 22) | (exp_run & 0x3FFFFF);
    return exp();
  };

  int run(int rno) {
    exp_run = (exp_run & 0xFFC00000) | (rno & 0x3FFFFF);
    return run();
  };

  int nevent() const {
    return (nevent_nboard >> 16);
  };

  int nboard() const {
    return (nevent_nboard & 0xFFFF);
  };

  int nevent(int nev) {
    nevent_nboard = ((nev & 0xFFFF) << 16) | (nevent_nboard & 0xFFFF);
    return nevent();
  };

  int nboard(int nb) {
    nevent_nboard = (nevent_nboard & 0xFFFF0000) | (nb & 0xFFFF);
    return nboard();
  };

  int nbyte() const {
    return nword * 4;
  };

  int nbyte_of_body() const {
    return nbyte() - sizeof(SendHeader) - sizeof(SendTrailer);
  };

  int nword_of_body() const {
    return (nbyte_of_body() / 4);
  };
};

#endif
