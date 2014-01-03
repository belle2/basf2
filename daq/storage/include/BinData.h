#ifndef _Belle2_BinData_h
#define _Belle2_BinData_h

#include <daq/slc/system/TCPSocket.h>

namespace Belle2 {

  struct BinTrailer {
    unsigned int reserved;
    unsigned int magic; /* should be 0x7FFF000X */
  };

  struct BinHeader {
    unsigned int nword; /* number of total words including header */
    unsigned int nword_in_header; /* number of words of this header == 6 */
    unsigned int nevent_nboard; /* number of board in this block, mostly # of COPPER */
    unsigned int exp_run;
    unsigned int event_number;
    unsigned int node_id;
  };

  class BinData {

  public:
    static const unsigned int TRAILER_MAGIC;
    static const unsigned int MAX_EVENT_WORDS = 2 * 1024 * 1024;

  public:
    BinData(void* buf = NULL);
    ~BinData() throw();

  public:
    int getExpNumber() const { return (_header->exp_run >> 22) & 0x3FF; };
    int getRunNumber() const { return (_header->exp_run & 0x3FFFFF); };
    int getEventNumber() const { return _header->event_number; };
    int setEventNumber(int number) { return _header->event_number = number; };
    int setExpNumber(int exp_no) {
      _header->exp_run = ((exp_no & 0x3FF) << 22) | (_header->exp_run & 0x3FFFFF);
      return getExpNumber();
    };
    int setRunNumber(int run_no) {
      _header->exp_run = (_header->exp_run & 0xFFC00000) | (run_no & 0x3FFFFF);
      return getRunNumber();
    };
    int getNEvent() const { return (_header->nevent_nboard >> 16); };
    int getNBoard() const { return (_header->nevent_nboard & 0xFFFF); };
    int setNEvent(int nev) {
      _header->nevent_nboard = ((nev & 0xFFFF) << 16) | (_header->nevent_nboard & 0xFFFF);
      return getNEvent();
    };
    int setNBoard(int nb) {
      _header->nevent_nboard = (_header->nevent_nboard & 0xFFFF0000) | (nb & 0xFFFF);
      return getNBoard();
    };
    int getWordSize() const { return _header->nword; };
    int getByteSize() const { return _header->nword * 4; };
    int getHeaderWordSize() const { return _header->nword_in_header; };
    int getHeaderByteSize() const { return _header->nword_in_header * 4; };
    int getBodyByteSize() const {
      return getByteSize() - sizeof(BinHeader) - sizeof(BinTrailer);
    }
    int getBodyWordSize() const { return (getBodyByteSize() / 4); };
    unsigned int getTrailerMagic() {
      _trailer = (BinTrailer*)(_body + getBodyWordSize());
      return _trailer->magic;
    }

  public:
    unsigned int recvEvent(TCPSocket& socket) throw(IOException);
    unsigned int sendEvent(TCPSocket& socket) const throw(IOException);
    int* getBuffer() { return _buf; }
    const int* getBuffer() const { return _buf; }
    void setBuffer(void* buf);

  public:
    BinHeader* getHeader() { return _header; };
    const BinHeader* getHeader() const { return _header; };
    BinTrailer* getTrailer() { return _trailer; }
    const BinTrailer* getTrailer() const { return _trailer; }
    unsigned int* getBody() { return _body; };
    const unsigned int* getBody() const { return _body; };

  protected:
    BinTrailer* _trailer;
    BinHeader* _header;
    unsigned int* _body;
    mutable int* _buf;

  };

}

#endif
