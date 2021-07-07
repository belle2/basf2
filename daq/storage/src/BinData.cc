/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/storage/BinData.h"

#include "daq/slc/base/IOException.h"

#include <cstdio>

using namespace Belle2;

const unsigned int BinData::TRAILER_MAGIC = 0x7FFF0007;

BinData::BinData(void* buf)
{
  if (buf != NULL) {
    setBuffer(buf);
    m_header->nword_in_header = sizeof(BinHeader) / 4;
    m_header->nword = (sizeof(BinHeader) + sizeof(BinTrailer)) / 4;
    m_header->nevent_nboard = 0;
    m_header->exp_run = 0;
    m_header->event_number = 0;
    m_header->node_id = 0;
  }
}

BinData::~BinData() noexcept
{

}

void BinData::setBuffer(void* buf)
{
  if (buf != NULL) {
    m_buf = (int*)buf;
    m_header = (BinHeader*)buf;
    m_body = (unsigned int*)(((char*)buf) + sizeof(BinHeader));
  } else {
    m_buf = NULL;
  }
}

unsigned int BinData::recvEvent(TCPSocket& socket)
{
  unsigned int count = socket.read(m_header, sizeof(BinHeader));
  if (m_header->nword_in_header * 4 != sizeof(BinHeader)) {
    throw (IOException("unexpected header size %d words",
                       m_header->nword_in_header));
  }
  const int nbytes_remains = getByteSize() - sizeof(BinHeader);
  count += socket.read(m_body, nbytes_remains);
  m_trailer = (BinTrailer*)(((char*)m_body) + nbytes_remains - sizeof(BinTrailer));
  if (m_trailer->magic != TRAILER_MAGIC) {
    throw (IOException("unexpected header trailer magic %08x",
                       m_trailer->magic));
  }
  return count;
}

unsigned int BinData::sendEvent(TCPSocket& socket) const
{
  return socket.write(m_buf, getByteSize());
}

void BinData::print() noexcept
{
  printf("headerwords=%d, nwords=%d, nboard=%d, nevent=%d, evtno=%d, expno=%d, runno=%d, subno=%d, nodeid=%d, trailer=%04x\n",
         getHeaderWordSize(),
         getWordSize(),
         //  getBodyWordSize(),
         getNBoard(),
         getNEvent(),
         getEventNumber(),
         getExpNumber(),
         getRunNumber(),
         getSubNumber(),
         getNodeId(),
         getTrailerMagic());
}
