/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ROIpayload.h>
#include <arpa/inet.h>
#include <boost/crc.hpp>
#include <stdio.h>
#include <framework/logging/Logger.h>

#define MAGIC_WORD (0xBE12DA7A)

using namespace Belle2;
using boost::crc_optimal;

ROIpayload::ROIpayload(int rois)
{
  init(HEADER_SIZE_WITH_LENGTH + 2 * rois + 1); // HEADER + 2*ROIS + CHECKSUM
};

void ROIpayload::init(int length)
{
  m_index = 0;
  m_length = length;
  if (length == 0)
    m_rootdata = NULL;
  else
    m_rootdata = new int[length];
  m_data32 = (uint32_t*)m_rootdata;
  m_data64 = (ROIrawID::baseType*)(m_data32 + HEADER_SIZE_WITH_LENGTH);
}

void ROIpayload::setPayloadLength(int length)
{
  m_data32[OFFSET_MAGIC] = htonl(MAGIC_WORD);
  m_data32[OFFSET_LENGTH] = htonl(length);
  m_packetLengthByte = length + 2 * sizeof(uint32_t);
}

void ROIpayload::setPayloadLength()
{
  unsigned int lengthInBytes =
    sizeof(uint32_t) +           // The header
    sizeof(uint32_t) +           // The trigger number
    sizeof(uint32_t) +     // Run Subrun Experiment Number
    m_index * sizeof(uint64_t) + // The rois
    sizeof(uint32_t);            // The CRC

  m_data32[OFFSET_MAGIC] = htonl(MAGIC_WORD);
  m_data32[OFFSET_LENGTH] = htonl(lengthInBytes);
  m_packetLengthByte = lengthInBytes + 2 * sizeof(uint32_t);  // The space for the magic word and payload length
}

void ROIpayload::setHeader(bool Accepted, bool SendAll, bool SendROIs)
{
  unsigned int h = 0xCAFE0000; // Magic
  if (Accepted) h |= 0x8000;
  if (SendAll) h |=
      0x4000; /// For Testbeam/Debugging purpose, we do not do any selection -> move to a downscaler on ONSEN Merger / SlowControl
  if (SendROIs) h |=
      0x2000; /// for debugging purpose, we want to see the ROIs which were send in (esp from DATCON) -> move to a downscaler on ONSEN Merger / SlowControl
  m_data32[OFFSET_HEADER] = htonl(h);
};

void ROIpayload::setTriggerNumber(unsigned long int triggerNumber)
{
  m_data32[OFFSET_TRIGNR] = htonl(triggerNumber);
};

void ROIpayload::setRunSubrunExpNumber(int run , int subrun , int exp)
{
  int rseNumber;
  rseNumber = (exp & 0x3FF) << 22 | (run & 0x3FFF) << 8 | (subrun & 0xFF) ;
  m_data32[OFFSET_RUNNR] = htonl(rseNumber);
};

//void ROIpayload::addROIraw(ROIrawID roiraw)
void ROIpayload::addROIraw(unsigned long int roiraw)
{
  if ((int*)(m_data64 + m_index) >= m_rootdata + m_length) {
    B2ERROR("Adding too many ROIs to the ROIpayload." << std::endl <<
            "Something really fishy is going on");
    return;
  }
  m_data64[m_index++] = roiraw;
}

void ROIpayload::setCRC()
{

  //  assert((int*)(m_data64 + m_index) ==  m_rootdata + m_length - 1);

  // TODO Pointer comparison is bad
  if ((int*)(m_data32 + m_index * 2 + HEADER_SIZE_WITH_LENGTH) >= m_rootdata + m_length) {
    B2ERROR("No space left on the ROI payload to write the CRC." << std::endl);
    return;
  }

  crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhh_crc_32;

  dhh_crc_32.process_bytes((void*)(m_rootdata + OFFSET_HEADER),
                           HEADER_SIZE_WO_LENGTH * sizeof(uint32_t) + m_index * sizeof(uint64_t));

  m_data32[OFFSET_ROIS + m_index * 2] = htonl(dhh_crc_32.checksum()) ;
}
