/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/unpacking/PXDMappingLookup.h>
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
  unsigned int iDCD = PXD::PXDMappingLookup::getDCDID(uid, vid, id) - 1;
  unsigned int iSWB = PXD::PXDMappingLookup::getSWBID(vid) - 1;
  unsigned short chipID = iDCD * 6 + iSWB;
  return m_gains.getGainCorrection(id.getID(), chipID);
}


void Belle2::PXD::PXDGainCalibrator::setGains()
{
  m_gains = **m_gainsFromDB;
}



