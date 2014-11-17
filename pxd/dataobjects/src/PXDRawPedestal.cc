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

#include <pxd/dataobjects/PXDRawPedestal.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;

PXDRawPedestal::PXDRawPedestal(VxdID sensorID, void* data):
  m_sensorID(sensorID), m_pedestals()
{
  unsigned char* d = (unsigned char*)data;
  m_dhp_header = ((boost::spirit::endian::ubig16_t*)data)[2];// swap endianess
  d += 8; // Skip DHH and DHP header, data is 128kb large (+ 8 bytes)
  for (unsigned int i = 0; i < sizeof(m_pedestals); i++) {
    m_pedestals[i] = d[2 * i + 1]; // Check if endianess is correctly done, seems so... TODO
  }
};

ClassImp(PXDRawPedestal)
