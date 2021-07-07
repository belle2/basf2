/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <vtx/reconstruction/VTXGainCalibrator.h>
#include <vxd/geometry/GeoCache.h>
#include <vtx/geometry/SensorInfo.h>
#include <framework/gearbox/Const.h>
#include <iostream>

using namespace std;


void Belle2::VTX::VTXGainCalibrator::initialize()
{
  m_gainsFromDB = unique_ptr<Belle2::DBObjPtr<Belle2::VTXGainMapPar>>(new Belle2::DBObjPtr<Belle2::VTXGainMapPar>());

  if ((*m_gainsFromDB).isValid()) {
    setGains();
    (*m_gainsFromDB).addCallback(this, &Belle2::VTX::VTXGainCalibrator::setGains);
  }
}


Belle2::VTX::VTXGainCalibrator& Belle2::VTX::VTXGainCalibrator::getInstance()
{
  static std::unique_ptr<Belle2::VTX::VTXGainCalibrator> instance(new Belle2::VTX::VTXGainCalibrator());
  return *instance;
}


float Belle2::VTX::VTXGainCalibrator::getGainCorrection(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  unsigned int rowsPerBin = Belle2::VXD::GeoCache::getInstance().get(id).getVCells() / m_gains.getBinsV();
  unsigned int drainsPerBin = 4 * Belle2::VXD::GeoCache::getInstance().get(id).getUCells() / m_gains.getBinsU();
  unsigned int uBin = (uid * 4 + vid % 4) / drainsPerBin;
  unsigned int vBin = vid / rowsPerBin;
  return m_gains.getContent(id.getID(), uBin, vBin);
}

float Belle2::VTX::VTXGainCalibrator::getADUToEnergy(Belle2::VxdID id, unsigned int uid, unsigned int vid) const
{
  Belle2::VTX::SensorInfo info = dynamic_cast<const Belle2::VTX::SensorInfo&>(Belle2::VXD::GeoCache::get(id));
  float ADCUnit = info.getADCunit();
  float Gq = info.getGq();
  return Const::ehEnergy * ADCUnit / Gq / getGainCorrection(id, uid, vid);
}



unsigned short Belle2::VTX::VTXGainCalibrator::getBinU(VxdID id, unsigned int uid, unsigned int vid, unsigned short nBinsU) const
{
  unsigned int drainsPerBin = 4 * Belle2::VXD::GeoCache::getInstance().get(id).getUCells() / nBinsU;
  return (uid * 4 + vid % 4) / drainsPerBin;
}

unsigned short Belle2::VTX::VTXGainCalibrator::getBinU(VxdID id, unsigned int uid, unsigned int vid) const
{
  return getBinU(id, uid, vid, m_gains.getBinsU());
}

unsigned short Belle2::VTX::VTXGainCalibrator::getBinV(VxdID id, unsigned int vid, unsigned short nBinsV) const
{
  unsigned int rowsPerBin = Belle2::VXD::GeoCache::getInstance().get(id).getVCells() / nBinsV;
  return vid / rowsPerBin;
}

unsigned short Belle2::VTX::VTXGainCalibrator::getBinV(VxdID id, unsigned int vid) const
{
  return getBinV(id, vid, m_gains.getBinsV());
}

unsigned short Belle2::VTX::VTXGainCalibrator::getGlobalID(VxdID id, unsigned int uid, unsigned int vid) const
{
  auto uBin = getBinU(id, uid, vid);
  auto vBin = getBinV(id, vid);
  return m_gains.getGlobalID(uBin, vBin);
}


void Belle2::VTX::VTXGainCalibrator::setGains()
{
  m_gains = **m_gainsFromDB;
}



