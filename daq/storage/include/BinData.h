/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
    ~BinData() noexcept;

  public:
    int getExpNumber() const { return (m_header->exp_run >> 22) & 0x3FF; };
    int getRunNumber() const { return (m_header->exp_run & 0x3FFFFF) >> 8; };
    int getSubNumber() const { return (m_header->exp_run & 0x3FFFFF) & 0xFF; };
    int getEventNumber() const { return m_header->event_number; };
    int setEventNumber(int number) { return m_header->event_number = number; };
    int setExpNumber(int exp_no)
    {
      m_header->exp_run = ((exp_no & 0x3FF) << 22) | (m_header->exp_run & 0x3FFFFF);
      return getExpNumber();
    };
    int setRunNumber(int run_no, int sub_no)
    {
      m_header->exp_run = (m_header->exp_run & 0xFFC00000) |
                          (run_no & 0x3FFFFF) << 8 | (sub_no & 0xFF);
      return getRunNumber();
    };
    int getNEvent() const { return (m_header->nevent_nboard >> 16); };
    int getNBoard() const { return (m_header->nevent_nboard & 0xFFFF); };
    int setNEvent(int nev)
    {
      m_header->nevent_nboard = ((nev & 0xFFFF) << 16) | (m_header->nevent_nboard & 0xFFFF);
      return getNEvent();
    };
    int setNBoard(int nb)
    {
      m_header->nevent_nboard = (m_header->nevent_nboard & 0xFFFF0000) | (nb & 0xFFFF);
      return getNBoard();
    };
    void setWordSize(int nword) { m_header->nword = nword; };
    int getWordSize() const { return m_header->nword; };
    int getByteSize() const { return m_header->nword * 4; };
    int getHeaderWordSize() const { return m_header->nword_in_header; };
    int getHeaderByteSize() const { return m_header->nword_in_header * 4; };
    int getBodyByteSize() const
    {
      return getByteSize() - sizeof(BinHeader) - sizeof(BinTrailer);
    }
    void setBodyWordSize(int nword)
    {
      setWordSize(nword + (sizeof(BinHeader) + sizeof(BinTrailer)) / 4);
      m_trailer = (BinTrailer*)(m_body + getBodyWordSize());
      m_trailer->magic = TRAILER_MAGIC;
    }
    int getBodyWordSize() const { return (getBodyByteSize() / 4); };
    unsigned int getTrailerMagic()
    {
      m_trailer = (BinTrailer*)(m_body + getBodyWordSize());
      return m_trailer->magic;
    }
    int getNodeId() const { return m_header->node_id; };
    void setNodeId(int id) { m_header->node_id = id; };

  public:
    unsigned int recvEvent(TCPSocket& socket);
    unsigned int sendEvent(TCPSocket& socket) const;
    int* getBuffer() { return m_buf; }
    const int* getBuffer() const { return m_buf; }
    void setBuffer(void* buf);
    void print() noexcept;

  public:
    BinHeader* getHeader() { return m_header; };
    const BinHeader* getHeader() const { return m_header; };
    BinTrailer* getTrailer() { return m_trailer; }
    const BinTrailer* getTrailer() const { return m_trailer; }
    unsigned int* getBody() { return m_body; };
    const unsigned int* getBody() const { return m_body; };

  protected:
    BinTrailer* m_trailer;
    BinHeader* m_header;
    unsigned int* m_body;
    mutable int* m_buf;

  };

}

#endif
