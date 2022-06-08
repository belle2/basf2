/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//! *************************************************************
//! Important Remark:
//! Up to now the format for Raw frames as well as size etc
//! is not well defined. It will most likely change!
//! E.g. not the whole mem is dumped, but only a part of it.
//! *************************************************************

#include <pxd/dataobjects/PXDRawAdc.h>
#include <boost/endian/arithmetic.hpp>
#include <framework/logging/Logger.h>
#include <cstring>

using namespace std;
using namespace Belle2;

PXDRawAdc::PXDRawAdc(VxdID sensorID, void* data, int len):
  m_sensorID(sensorID), m_adcs()
{
  unsigned char* d = (unsigned char*)data;
  m_dhp_header = ((boost::endian::big_uint16_t*)data)[2];
  d += 8; // Skip DHH and DHP header, data is 64kb large (+ 8 bytes)
  len -= 8;
  if (len < 0) {
    B2ERROR("PXDRawAdc size is negative!");
  } else {
    m_adcs.resize(len);// resize vector
    std::memcpy(&m_adcs[0], d, len);// lowlevel hardcore, TODO maybe better use std::copy ?
    // seems endianess swapping is not needed
  }
};
