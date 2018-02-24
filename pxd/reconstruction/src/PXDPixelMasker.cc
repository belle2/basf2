/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <pxd/reconstruction/PXDPixelMasker.h>
#include <vxd/geometry/GeoCache.h>
#include <memory>

using namespace std;


Belle2::PXD::PXDPixelMasker& Belle2::PXD::PXDPixelMasker::getInstance()
{
  static std::unique_ptr<Belle2::PXD::PXDPixelMasker> instance(new Belle2::PXD::PXDPixelMasker());
  return *instance;
}


void Belle2::PXD::PXDPixelMasker::maskSinglePixel(Belle2::VxdID id, unsigned int uid, unsigned int vid)
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().get(id).getVCells();
  m_maskedPixelPar.maskSinglePixel(id.getID(), uid * vCells + vid);
}


bool Belle2::PXD::PXDPixelMasker::pixelOK(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().get(id).getVCells();
  return m_maskedPixelPar.pixelOK(id.getID(), uid * vCells + vid);
}


