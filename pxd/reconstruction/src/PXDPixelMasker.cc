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


using namespace std;


void Belle2::PXD::PXDPixelMasker::initialize()
{
  m_maskedPixelsFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::PXDMaskedPixelPar>>(new Belle2::DBObjPtr<Belle2::PXDMaskedPixelPar>());

  if ((*m_maskedPixelsFromDB).isValid()) {
    setMaskedPixels();
    (*m_maskedPixelsFromDB).addCallback(this, &Belle2::PXD::PXDPixelMasker::setMaskedPixels);
  }

  m_deadPixelsFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::PXDDeadPixelPar>>(new Belle2::DBObjPtr<Belle2::PXDDeadPixelPar>());

  if ((*m_deadPixelsFromDB).isValid()) {
    setDeadPixels();
    (*m_deadPixelsFromDB).addCallback(this, &Belle2::PXD::PXDPixelMasker::setDeadPixels);
  }
}


Belle2::PXD::PXDPixelMasker& Belle2::PXD::PXDPixelMasker::getInstance()
{
  static std::unique_ptr<Belle2::PXD::PXDPixelMasker> instance(new Belle2::PXD::PXDPixelMasker());
  return *instance;
}


void Belle2::PXD::PXDPixelMasker::maskSinglePixel(Belle2::VxdID id, unsigned int uid, unsigned int vid)
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().get(id).getVCells();
  m_maskedPixels.maskSinglePixel(id.getID(), uid * vCells + vid);
}



bool Belle2::PXD::PXDPixelMasker::pixelOK(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().get(id).getVCells();
  return m_maskedPixels.pixelOK(id.getID(), uid * vCells + vid);
}

bool Belle2::PXD::PXDPixelMasker::pixelDead(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  auto sensorID = id.getID();
  if (m_deadPixels.isDeadSensor(sensorID))
    return true;

  if (m_deadPixels.isDeadRow(sensorID, vid))
    return true;

  if (m_deadPixels.isDeadDrain(sensorID, uid * 4 + vid % 4))
    return true;

  auto vCells = Belle2::VXD::GeoCache::getInstance().get(id).getVCells();
  if (m_deadPixels.isDeadSinglePixel(sensorID, uid * vCells + vid))
    return true;

  return false;
}


void Belle2::PXD::PXDPixelMasker::setMaskedPixels()
{
  m_maskedPixels = **m_maskedPixelsFromDB;
}

void Belle2::PXD::PXDPixelMasker::setDeadPixels()
{
  m_deadPixels = **m_deadPixelsFromDB;
}


