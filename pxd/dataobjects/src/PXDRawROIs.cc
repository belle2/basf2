/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/PXDRawROIs.h>
#include <boost/endian/arithmetic.hpp>

using namespace std;
using namespace Belle2;

PXDRawROIs::PXDRawROIs(unsigned int nrroi, unsigned int* data):
  m_2timesNrROIs(2 * nrroi), m_rois(NULL)
{
  m_rois = new int[m_2timesNrROIs];
  auto* d = (boost::endian::big_uint32_t*)data;
  for (unsigned int i = 0; i < m_2timesNrROIs; i++) {
    m_rois[i] = d[i];// We have to do 32bit endian swap, TODO VERIFY this!
  }
}

PXDRawROIs::~PXDRawROIs()
{
  if (m_rois)
    delete[] m_rois;
}
