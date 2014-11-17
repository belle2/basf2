/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//! *************************************************************
//! Important Remark:
//! Up to now the format for Raw frames as well as size etc
//! is not well defined. It will most likely change!
//! E.g. not the whole mem is dumped, but only a part of it.
//! *************************************************************

#include <pxd/dataobjects/PXDRawAdc.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;

PXDRawAdc::PXDRawAdc(VxdID sensorID, void* data, bool pedestal_flag):
  m_sensorID(sensorID) , m_adcs()
{
  unsigned char* d = (unsigned char*)data;
  m_dhp_header = ((boost::spirit::endian::ubig16_t*)data)[2];
  d += 8; // Skip DHH and DHP header, data is 64kb large (+ 8 bytes)
  if (pedestal_flag) {
    for (unsigned int i = 0; i < sizeof(m_adcs); i++) {
      m_adcs[i] = d[2 * i + 0]; // Check if endianess is correctly done... TODO , seems so
    }
  } else {
    memcpy(m_adcs, d, sizeof(m_adcs)); // check if we need to swap endianess TODO , seems we do not
//         for (unsigned int i = 0; i < sizeof(m_adcs); i++) {
//           m_adcs[i] = d[i]; // swap endianess --- again ---
//         }
  }
};

ClassImp(PXDRawAdc)
