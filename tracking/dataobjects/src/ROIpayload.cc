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
//#define _BSD_SOURCE
//#include <endian.h>

using namespace Belle2;
using boost::crc_optimal;

ROIpayload::ROIpayload(int length)
{
  init(length);
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
  m_data64 = (ROIrawID::baseType*)(m_data32 + 3);
}

void ROIpayload::setPayloadLength(int length)
{

  //  m_data32[0] = htobe32(length);
  m_data32[0] = htonl(length);


  m_packetLengthByte = length + sizeof(uint32_t);
}

void ROIpayload::setPayloadLength()
{
  unsigned int lengthInBytes =
    sizeof(uint32_t) +           // The header
    sizeof(uint32_t) +           // The trigger number
    m_index * sizeof(uint64_t) + // The rois
    sizeof(uint32_t);            // The CRC

  m_data32[0] = htonl(lengthInBytes);
  m_packetLengthByte =
    lengthInBytes +
    sizeof(uint32_t);  // The space for payload lenght

}

void ROIpayload::setHeader()
{

  //  m_data32[1] = htobe32(0xCAFE8000);
  //  TODO
  //  warning: use of logical '||' with constant operand [-Wconstant-logical-operand]
  //
  //  m_data32[1] = htonl(0xCAFE8000 || 1 << 13);
  m_data32[1] = htonl(0xCAFE8000 << 13);

};

void ROIpayload::setTriggerNumber(unsigned long int triggerNumber)
{

  //  m_data32[2] = htobe32(triggerNumber);
  m_data32[2] = htonl(triggerNumber);

};

//void ROIpayload::addROIraw(ROIrawID roiraw)
void ROIpayload::addROIraw(unsigned long int roiraw)
{

  if ((int*)(m_data64 + m_index) >= m_rootdata + m_length) {
    B2ERROR("Adding too many ROIs to the ROIpayload." << std::endl <<
            "Something really fishy is going on");
    return;
  }

  m_data64[m_index++] = roiraw; //.getBigEndian();

}

void ROIpayload::setCRC()
{

  //  assert((int*)(m_data64 + m_index) ==  m_rootdata + m_length - 1);

  if ((int*)(m_data32 + m_index * 2 + 3) >= m_rootdata + m_length) {
    B2ERROR("No space left on the ROI payload to write the CRC." << std::endl);
    return;
  }

  crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhh_crc_32;

  dhh_crc_32.process_bytes((void*)(m_rootdata + 1), 2 * sizeof(uint32_t) + m_index * sizeof(uint64_t));

  //  m_data32[m_index * 2 + 3] = htobe32(dhh_crc_32.checksum()) ;
  m_data32[m_index * 2 + 3] = htonl(dhh_crc_32.checksum()) ;


}

ClassImp(ROIpayload);
