/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach
 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/dataobjects/PXDHardwareCluster.h>
#include <boost/spirit/home/support/detail/endian.hpp>

using namespace std;
using namespace Belle2;

PXDHardwareCluster::PXDHardwareCluster(unsigned short* pixelU, unsigned short* pixelV, unsigned char* pixelQ, unsigned int nrPixel,
                                       unsigned char seedCharge,
                                       unsigned short seedU, unsigned short seedV, unsigned int clusterCharge, VxdID vxdID, unsigned int chipID):
  m_nrPixel(nrPixel), m_pixelU(0), m_pixelV(0), m_pixelQ(0), m_seedCharge(seedCharge), m_seedU(seedU), m_seedV(seedV),
  m_clusterCharge(clusterCharge), m_vxdID(vxdID), m_chipID(chipID)
{
  m_pixelU = new unsigned short[m_nrPixel];
  m_pixelV = new unsigned short[m_nrPixel];
  m_pixelQ = new unsigned char[m_nrPixel];
  for (unsigned int i = 0 ; i < m_nrPixel ; i++) {
    m_pixelU[i] = pixelU[i]; //We have to do 16bit endian swap, TODO VERIFY this!
    m_pixelV[i] = pixelV[i]; //We have to do 16bit endian swap, TODO VERIFY this!
    m_pixelQ[i] = pixelQ[i]; //We have to do 16bit endian swap, TODO VERIFY this!
  }
};

PXDHardwareCluster::~PXDHardwareCluster()
{
  delete m_pixelU;
  delete m_pixelV;
  delete m_pixelQ;
};
