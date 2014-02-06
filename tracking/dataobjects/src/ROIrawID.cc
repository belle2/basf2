/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arpa/inet.h>
#include <tracking/dataobjects/ROIrawID.h>
using namespace Belle2;

ROIrawID::baseType ROIrawID::getBigEndian() const
{
  //  return ROIrawID::baseType htobe64(m_rawID.id);

  uint32_t low_host, hi_host;
  uint64_t low_be, hi_be;

  /* Return immediately if byte order is big endian. */
  if (htonl(0x01020304) == 0x01020304)
    return m_rawID.id;
  /* Change byte order in low and high words. */
  low_host = m_rawID.id & 0x00000000FFFFFFFF;
  hi_host = (m_rawID.id & 0xFFFFFFFF00000000) >> 32;
  low_be = htonl(low_host);
  hi_be = htonl(hi_host);
  /* Exchange low and high words. */
  return (ROIrawID::baseType)(low_be << 32 | hi_be);
}
