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
#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

using namespace Belle2;
using namespace std;


ROIDetPlane::ROIDetPlane(const VxdID& sensorInfo)
  : m_sensorPhiMin(0)
  , m_sensorPhiMax(2 * M_PI)
  , m_sensorZMin(-10)
  , m_sensorZMax(10)
  , m_layer(0)
  , m_sensorInfo(sensorInfo)
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  TVector3 local(0, 0, 0);
  TVector3 uVector(1, 0, 0);
  TVector3 vVector(0, 1, 0);

  const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(m_sensorInfo);
  TVector3 globalSensorPos = aSensorInfo.pointToGlobal(local, true);
  TVector3 globaluVector = aSensorInfo.vectorToGlobal(uVector, true);
  TVector3 globalvVector = aSensorInfo.vectorToGlobal(vVector, true);

  setO(globalSensorPos);

  setUV(globaluVector, globalvVector);

  m_layer = (aSensorInfo.getID()).getLayerNumber();
}

ROIDetPlane::ROIDetPlane(const VxdID& sensorInfo, double toleranceZ, double tolerancePhi)
  : m_sensorInfo(sensorInfo)
{

  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();

  TVector3 local(0, 0, 0);
  TVector3 uVector(1, 0, 0);
  TVector3 vVector(0, 1, 0);

  const VXD::SensorInfoBase& aSensorInfo = aGeometry.getSensorInfo(m_sensorInfo);
  TVector3 globalSensorPos = aSensorInfo.pointToGlobal(local, true);
  TVector3 globaluVector = aSensorInfo.vectorToGlobal(uVector, true);
  TVector3 globalvVector = aSensorInfo.vectorToGlobal(vVector, true);

  setO(globalSensorPos);

  setUV(globaluVector, globalvVector);

  //settin acceptance for the sensor
  TVector3 minVecV(0, -aSensorInfo.getVSize() / 2.0, 0);
  TVector3 minVecU(-aSensorInfo.getUSize() / 2.0, 0, 0);
  TVector3 maxVecV(0, aSensorInfo.getVSize() / 2.0, 0);
  TVector3 maxVecU(aSensorInfo.getUSize() / 2.0, 0, 0);

  m_sensorPhiMin = std::atan2(aSensorInfo.pointToGlobal(minVecU, true).Y(),
                              aSensorInfo.pointToGlobal(minVecU, true).X()) - tolerancePhi;
  m_sensorPhiMax = std::atan2(aSensorInfo.pointToGlobal(maxVecU, true).Y(),
                              aSensorInfo.pointToGlobal(maxVecU, true).X()) + tolerancePhi;
  if (m_sensorPhiMin < -M_PI) m_sensorPhiMin = m_sensorPhiMin + 2.0 * M_PI; // correct 'underflows' of definition
  if (m_sensorPhiMax > M_PI) m_sensorPhiMax = m_sensorPhiMax - 2.0 * M_PI; // correct 'overflows' of definition

  m_sensorZMin = aSensorInfo.pointToGlobal(minVecV, true).Z() - toleranceZ;
  m_sensorZMax = aSensorInfo.pointToGlobal(maxVecV, true).Z() + toleranceZ;

  m_layer = (aSensorInfo.getID()).getLayerNumber();

}


ROIDetPlane::~ROIDetPlane() {}


bool ROIDetPlane::isSensorInRange(TVector3 trackPosition, int layer)
{

  double trackPhi = std::atan2(trackPosition.Y(), trackPosition.X());
  double trackZ = trackPosition.Z();

  // check whether genfit track is in range
  if (not(m_sensorZMin <= trackZ && m_sensorZMax >= trackZ)) {
    return false;
  }
  // phi jumps from pi to -pi, in this case min>max and the processing has to differ
  if (m_sensorPhiMin < m_sensorPhiMax) { // standard case
    if (not(m_sensorPhiMin <= trackPhi && m_sensorPhiMax >= trackPhi)) {
      return false;
    }
  } else { // on pi->-pi
    if (not(m_sensorPhiMin <= trackPhi && M_PI >= trackPhi) && not(-M_PI <= trackPhi && m_sensorPhiMax >= trackPhi)) {
      return false;
    }
  }
  if (layer != m_layer)
    return false;

  return true;
}
