/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <pxd/reconstruction/PXDPixelMasker.h>
#include <vxd/geometry/GeoCache.h>


using namespace std;


void Belle2::PXD::PXDPixelMasker::initialize()
{
  m_pixelThresholdsFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::PXDPixelThresholdPar>>(new
                            Belle2::DBObjPtr<Belle2::PXDPixelThresholdPar>());

  if ((*m_pixelThresholdsFromDB).isValid()) {
    setPixelThresholds();
    (*m_pixelThresholdsFromDB).addCallback(this, &Belle2::PXD::PXDPixelMasker::setPixelThresholds);
  }

  m_maskedPixelsFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::PXDMaskedPixelPar>>(new Belle2::DBObjPtr<Belle2::PXDMaskedPixelPar>());

  if ((*m_maskedPixelsFromDB).isValid()) {
    setMaskedPixels();
    (*m_maskedPixelsFromDB).addCallback(this, &Belle2::PXD::PXDPixelMasker::setMaskedPixels);
    if (!(*m_pixelThresholdsFromDB).isValid() or !(**m_pixelThresholdsFromDB).getPixelThresholdMap().size()) {
      setPixelThresholds(m_maskedPixels);
      (*m_pixelThresholdsFromDB).addCallback(this, &Belle2::PXD::PXDPixelMasker::setPixelThresholds);
    }
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

void Belle2::PXD::PXDPixelMasker::setSinglePixelThreshold(Belle2::VxdID id, unsigned int uid, unsigned int vid,
                                                          unsigned short pixThr)
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().getSensorInfo(id).getVCells();
  m_pixelThresholds.setSinglePixelThreshold(id.getID(), uid * vCells + vid, pixThr);
}

void Belle2::PXD::PXDPixelMasker::maskSinglePixel(Belle2::VxdID id, unsigned int uid, unsigned int vid)
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().getSensorInfo(id).getVCells();
  m_maskedPixels.maskSinglePixel(id.getID(), uid * vCells + vid);
}

unsigned short Belle2::PXD::PXDPixelMasker::getPixelThreshold(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().getSensorInfo(id).getVCells();
  return m_pixelThresholds.getPixelThreshold(id.getID(), uid * vCells + vid);
}

bool Belle2::PXD::PXDPixelMasker::pixelOK(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  auto vCells = Belle2::VXD::GeoCache::getInstance().getSensorInfo(id).getVCells();
  return (m_pixelThresholds.getPixelThreshold(id.getID(), uid * vCells + vid) < 255);
  //return m_maskedPixels.pixelOK(id.getID(), uid * vCells + vid);
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

  auto vCells = Belle2::VXD::GeoCache::getInstance().getSensorInfo(id).getVCells();
  if (m_deadPixels.isDeadSinglePixel(sensorID, uid * vCells + vid))
    return true;

  return false;
}

void Belle2::PXD::PXDPixelMasker::setPixelThresholds(PXDMaskedPixelPar maskedPixels)
{
  auto maskedPixelMap = maskedPixels.getMaskedPixelMap();
  for (auto maskedSingles = maskedPixelMap.begin(); maskedSingles != maskedPixelMap.end(); maskedSingles++) {
    auto sensorID = maskedSingles->first;
    auto singles = maskedSingles->second;
    for (auto single = singles.begin(); single != singles.end(); single++) {
      m_pixelThresholds.setSinglePixelThreshold(sensorID, *single, 255);
    }
  }
}

void Belle2::PXD::PXDPixelMasker::setPixelThresholds()
{
  m_pixelThresholds = **m_pixelThresholdsFromDB;
}

void Belle2::PXD::PXDPixelMasker::setMaskedPixels()
{
  m_maskedPixels = **m_maskedPixelsFromDB;
}

void Belle2::PXD::PXDPixelMasker::setDeadPixels()
{
  m_deadPixels = **m_deadPixelsFromDB;
}


