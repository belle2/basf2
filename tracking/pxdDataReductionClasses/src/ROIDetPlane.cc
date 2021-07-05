/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/pxdDataReductionClasses/ROIDetPlane.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <framework/logging/Logger.h>

using namespace Belle2;
using namespace std;


ROIDetPlane::ROIDetPlane(const VxdID& vxdID) : ROIDetPlane(vxdID, 10.0, 0.4999999 * M_PI)
{
}


ROIDetPlane::ROIDetPlane(const VxdID& vxdID, double toleranceZ, double tolerancePhi)
  : m_vxdID(vxdID), m_orthoVec_upper(0, 0, 0), m_orthoVec_lower(0, 0, 0)
{

  const VXD::SensorInfoBase& aSensorInfo = VXD::GeoCache::getInstance().getSensorInfo(
                                             m_vxdID);   /**< reference to sensor info in geometry */

  TVector3 local(0, 0, 0);
  TVector3 uVector(1, 0, 0);
  TVector3 vVector(0, 1, 0);

  TVector3 globalSensorPos = aSensorInfo.pointToGlobal(local, true);
  TVector3 globaluVector = aSensorInfo.vectorToGlobal(uVector, true);
  TVector3 globalvVector = aSensorInfo.vectorToGlobal(vVector, true);

  setO(globalSensorPos);

  setUV(globaluVector, globalvVector);


  // the maximum distance in  u-direction for hit to be considered to lie on the sensor (for wedge take the maximum width)
  double maxDistU = std::max(aSensorInfo.getForwardWidth(),  aSensorInfo.getBackwardWidth()) / 2.0;

  // translate the phi-tolerance into a tolerance in u, NOTE: this is only approximate as it uses the center and not the edge of the sensor.
  if (tolerancePhi >= 0 && tolerancePhi < M_PI / 2.0) {
    maxDistU += fabs(std::tan(tolerancePhi) * globalSensorPos.Perp());
  } else {
    B2WARNING("No valid value for the phi tolerance given! Will use 0 tolerance!" << LogVar("tolerance phi", tolerancePhi));
  }


  // get points at upper and lower edge of the sensor
  TVector3 edgepoint_upper = globalSensorPos + maxDistU * globaluVector;
  TVector3 edgepoint_lower = globalSensorPos - maxDistU * globaluVector;
  /* Get the orthogonal vectors, no need to normalize as we only test for the sign.
     These two vectors are defined so that they are orthogonal to  the plane spanned by the z-axis and the vector going from
     the origin to the upper/lower edge of the sensor (global coordinates).*/
  m_orthoVec_upper = TVector3(0, 0, 1).Cross(edgepoint_upper);
  m_orthoVec_lower = TVector3(0, 0, 1).Cross(edgepoint_lower);

  //setting acceptance in z for the sensor
  TVector3 minVecV(0, -aSensorInfo.getVSize() / 2.0, 0);
  TVector3 maxVecV(0, aSensorInfo.getVSize() / 2.0, 0);
  m_sensorZMin = aSensorInfo.pointToGlobal(minVecV, true).Z() - toleranceZ;
  m_sensorZMax = aSensorInfo.pointToGlobal(maxVecV, true).Z() + toleranceZ;

  m_layer = (aSensorInfo.getID()).getLayerNumber();
}


bool ROIDetPlane::isSensorInRange(const TVector3& trackPosition, int layer)
{
  // check for correct layer
  if (layer != m_layer)
    return false;

  double trackZ = trackPosition.Z();
  // check whether genfit track is in z range
  if (m_sensorZMin > trackZ || m_sensorZMax < trackZ) {
    return false;
  }

  /* Positions on the sensor have to lie between the planes that go through the upper edge of the sensor
     and the lower edge of the senor (all w.r.t. the origin 0,0,0, and in r-phi-direction). That means for vectors pointing to the sensor
     it has to be "below" (w.r.t. the plane) the plane going through the upper edge defined by its
     orthogonal vector m_orthoVec_upper and
     the origin (which means the dot product of m_orthoVec_upper and the position is smaller 0) and "above" the lower plane
     defined by its orthogonal vector m_orthoVec_lower vector and the origin (which means the dot product with m_orthoVec_lower
     is greater 0) */
  if (trackPosition.Dot(m_orthoVec_upper) > 0 || trackPosition.Dot(m_orthoVec_lower) < 0) {
    return false;
  }

  // fullfilled all conditions
  return true;
}
