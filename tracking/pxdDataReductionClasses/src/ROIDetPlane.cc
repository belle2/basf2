/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <vxd/geometry/GeoCache.h>
//#include <tracking/dataobjects/ROIDetPlane.h>
#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

using namespace Belle2;
using namespace std;


ROIDetPlane::ROIDetPlane(const VxdID& sensorInfo)
  : m_sensorInfo(sensorInfo)
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  TVector3 local(0, 0, 0);
  TVector3 uVector(1, 0, 0);
  TVector3 vVector(0, 1, 0);

  const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(m_sensorInfo);
  TVector3 globalSensorPos = aSensorInfo.pointToGlobal(local);
  TVector3 globaluVector = aSensorInfo.vectorToGlobal(uVector);
  TVector3 globalvVector = aSensorInfo.vectorToGlobal(vVector);

  setO(globalSensorPos);

  setUV(globaluVector, globalvVector);

}

ROIDetPlane::~ROIDetPlane() {}
