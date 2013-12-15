#include "daq/storage/ONSENBinData.h"

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/Debugger.h>

#include <arpa/inet.h>
#include <cstring>

using namespace Belle2;

ONSENBinData::ONSENBinData(void* buf) : BinData(buf)
{

}

ONSENBinData::~ONSENBinData() throw()
{

}

unsigned int ONSENBinData::getTrigger() const
{
  return (ntohs(_start_frame->trigger_hi) << 16)
         + ntohs(_start_frame->trigger_lo);
}

unsigned int ONSENBinData::getTimetag() const
{
  return (ntohs(_start_frame->timetag_hi) << 16)
         + ntohs(_start_frame->timetag_lo);
}

unsigned int ONSENBinData::getEventNumber() const
{
  unsigned int nframe = getFrameNumber();
  if (nframe > MAX_PXD_FRAMES) return 0;
  _start_frame = (StartFrame*)(_body + nframe + 2);
  return getTrigger();
}

unsigned int ONSENBinData::getFrameNumber() const
{
  return ntohl(_body[1]);
}

unsigned int ONSENBinData::getFrameByteSize() const
{
  const unsigned int nframe = getFrameNumber();
  unsigned int nbyte = 0;
  for (unsigned int i = 0; i < nframe; i++) {
    nbyte += ntohl(_body[2 + i]);
  }
  if (_body[2 + nframe] != ntohs(0x3000)) {
    return nbyte - 8;
  } else {
    return nbyte;
  }
}

unsigned int ONSENBinData::getONSENMagic() const
{
  return ntohl(_body[0]);
}

