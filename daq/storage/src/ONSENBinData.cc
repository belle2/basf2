/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/storage/ONSENBinData.h"

#include <arpa/inet.h>

using namespace Belle2;

ONSENBinData::ONSENBinData(void* buf) : BinData(buf)
{

}

ONSENBinData::~ONSENBinData() throw()
{

}

unsigned int ONSENBinData::getTrigger() const
{
  return (ntohs(m_start_frame->trigger_hi) << 16)
         + ntohs(m_start_frame->trigger_lo);
}

unsigned int ONSENBinData::getTimetag() const
{
  uint64_t retval =
    ((uint64_t)ntohs(m_start_frame->time_tag_hi) << 32)
    | ((uint64_t)ntohs(m_start_frame->time_tag_mid) << 16)
    | (uint64_t)ntohs(m_start_frame->time_tag_lo_and_type);
  return (retval >> 4);
}

unsigned int ONSENBinData::getEventNumber() const
{
  unsigned int nframe = getFrameNumber();
  if (nframe > MAX_PXD_FRAMES) return 0;
  m_start_frame = (sose_frame_t*)(m_body + nframe + 2);
  return getTrigger();
}

unsigned int ONSENBinData::getTriggerType() const
{
  return (ntohs(m_start_frame->time_tag_lo_and_type) & 0xF);
}

unsigned int ONSENBinData::getFrameNumber() const
{
  return ntohl(m_body[1]);
}

unsigned int ONSENBinData::getFrameByteSize() const
{
  const unsigned int nframe = getFrameNumber();
  unsigned int nbyte = 0;
  for (unsigned int i = 0; i < nframe; i++) {
    nbyte += ntohl(m_body[2 + i]);
  }
  if (m_body[2 + nframe] != ntohs(0x3000)) {
    return nbyte - 8;
  } else {
    return nbyte;
  }
}

unsigned int ONSENBinData::getONSENMagic() const
{
  return ntohl(m_body[0]);
}

