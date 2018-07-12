/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/reconstruction/PXDGainCalibrator.h>
#include <vxd/geometry/GeoCache.h>

using namespace std;


void Belle2::PXD::PXDGainCalibrator::initialize()
{
  m_gainsFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::PXDGainMapPar>>(new Belle2::DBObjPtr<Belle2::PXDGainMapPar>());

  if ((*m_gainsFromDB).isValid()) {
    setGains();
    (*m_gainsFromDB).addCallback(this, &Belle2::PXD::PXDGainCalibrator::setGains);
  }
}


Belle2::PXD::PXDGainCalibrator& Belle2::PXD::PXDGainCalibrator::getInstance()
{
  static std::unique_ptr<Belle2::PXD::PXDGainCalibrator> instance(new Belle2::PXD::PXDGainCalibrator());
  return *instance;
}


float Belle2::PXD::PXDGainCalibrator::getGainCorrection(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  unsigned int rowsPerBin = Belle2::VXD::GeoCache::getInstance().get(id).getVCells() / m_gains.getCorrectionsV();
  unsigned int drainsPerBin = 4 * Belle2::VXD::GeoCache::getInstance().get(id).getUCells() / m_gains.getCorrectionsU();
  unsigned int uBin = (uid * 4 + vid % 4) / drainsPerBin;
  unsigned int vBin = vid / rowsPerBin;
  return m_gains.getGainCorrection(id.getID(), uBin, vBin);
}


unsigned short Belle2::PXD::PXDGainCalibrator::getBinU(VxdID id, unsigned int uid, unsigned int vid, unsigned short nBinsU) const
{
  unsigned int drainsPerBin = 4 * Belle2::VXD::GeoCache::getInstance().get(id).getUCells() / nBinsU;
  return (uid * 4 + vid % 4) / drainsPerBin;
}

unsigned short Belle2::PXD::PXDGainCalibrator::getBinU(VxdID id, unsigned int uid, unsigned int vid) const
{
  return getBinU(id, uid, vid, m_gains.getCorrectionsU());
}

unsigned short Belle2::PXD::PXDGainCalibrator::getBinV(VxdID id, unsigned int vid, unsigned short nBinsV) const
{
  unsigned int rowsPerBin = Belle2::VXD::GeoCache::getInstance().get(id).getVCells() / nBinsV;
  return vid / rowsPerBin;
}

unsigned short Belle2::PXD::PXDGainCalibrator::getBinV(VxdID id, unsigned int vid) const
{
  return getBinV(id, vid, m_gains.getCorrectionsV());
}

unsigned short Belle2::PXD::PXDGainCalibrator::getGainID(VxdID id, unsigned int uid, unsigned int vid) const
{
  auto uBin = getBinU(id, uid, vid);
  auto vBin = getBinV(id, vid);
  return m_gains.getGainID(uBin, vBin);
}


void Belle2::PXD::PXDGainCalibrator::setGains()
{
  m_gains = **m_gainsFromDB;
}



