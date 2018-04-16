/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/DATCONSVDSpacePoint.h>

using namespace std;
using namespace Belle2;

//---SVD related constructor---
DATCONSVDSpacePoint::DATCONSVDSpacePoint(std::vector<const DATCONSimpleSVDCluster*>& clusters,
                                         const VXD::SensorInfoBase* aSensorInfo)
{
  //---The following contains only sanity checks without effect, if nobody gave buggy information---
  //We have 1 or two SVD Clusters.
  B2ASSERT("You have to insert 1 or two SVD Clusters, but gave: " << clusters.size(), ((clusters.size() == 1)
           || (clusters.size() == 2)));

  //No cluster pointer is a nullptr.
  for (auto && cluster : clusters) {
    B2ASSERT("An DATCONSimpleSVDCluster Pointer is a nullptr!", cluster != nullptr);
  }

  //In case of 2 clusters, they are compatible with each other.
  if (clusters.size() == 2) {
    B2ASSERT("Clusters are on different Sensors.",   clusters[0]->getSensorID() == clusters[1]->getSensorID());
    B2ASSERT("Clusters are of same direction type.", clusters[0]->isUCluster()  != clusters[1]->isUCluster());
  }
  //---End sanity checks---

  m_vxdID = clusters[0]->getSensorID();

  //We need some handle to translate IDs to local and global coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  m_sensorType = aSensorInfo->getType();

  // retrieve position and sigma-values
  double uCoord =  0; // 0 = center of Sensor
  double vCoord =  0; // 0 = center of Sensor

  const DATCONSimpleSVDCluster* vCluster(NULL), *uCluster(NULL);
  for (const DATCONSimpleSVDCluster* aCluster : clusters) {
    if (aCluster->isUCluster() == true) {
      m_clustersAssigned.first = true;
      uCoord = aCluster->getPosition();
      uCluster = aCluster;
    } else {
      m_clustersAssigned.second = true;
      vCoord = aCluster->getPosition();
      vCluster = aCluster;
    }
  }

  if (aSensorInfo->getBackwardWidth() > aSensorInfo->getForwardWidth() &&
      vCluster != NULL && uCluster != NULL) // is a WedgeSensor and we do have a vCluster
    uCoord = uCluster->getPosition(vCoord);

  m_position = aSensorInfo->pointToGlobal(TVector3(uCoord, vCoord, 0));
  m_normalizedLocal = convertLocalToNormalizedCoordinates({ uCoord, vCoord } , m_vxdID, aSensorInfo);

}


std::pair<double, double> DATCONSVDSpacePoint::convertLocalToNormalizedCoordinates(
  const std::pair<double, double>& hitLocal, VxdID vxdID,
  const VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  //As the 0 is in the middle of sensor in the geometry, and we want
  // to normalize all positions to numbers between [0,1],
  // where the middle will be 0.5,
  // we need to do some calculation.
  double sensorSizeU =  aSensorInfo->getUSize(hitLocal.second); // this deals with the case of trapezoidal sensors too
  double sensorSizeV =  aSensorInfo->getVSize();

  double normalizedUPosition = (hitLocal.first +  0.5 * sensorSizeU) /
                               sensorSizeU; // indepedent of the trapezoidal sensor-issue by definition
  double normalizedVPosition = (hitLocal.second +  0.5 * sensorSizeV) / sensorSizeV;

  boundaryEnforce(normalizedUPosition, normalizedVPosition, 0, 1 , 0, vxdID);
  boundaryEnforce(normalizedVPosition, normalizedUPosition, 0, 1, 1, vxdID);

  return { normalizedUPosition, normalizedVPosition };
}



std::pair<double, double> DATCONSVDSpacePoint::convertNormalizedToLocalCoordinates(
  const std::pair<double, double>& hitNormalized, VxdID vxdID,
  const VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  // normalized coordinate range is from 0 to 1
  // local coordinate range is from - halfSensorSize to + halfSensorSize
  double localVPosition = (hitNormalized.second - 0.5) * aSensorInfo->getVSize();
  double uSizeAtHit = aSensorInfo->getUSize(localVPosition);
  double localUPosition = (hitNormalized.first - 0.5) * uSizeAtHit;

  boundaryEnforce(localVPosition, localUPosition,
                  -0.5 * aSensorInfo->getVSize(), 0.5 * aSensorInfo->getVSize(), 1, vxdID); // restrain hits to sensor boundaries

  boundaryEnforce(localUPosition, localVPosition, -0.5 * uSizeAtHit, 0.5 * uSizeAtHit,
                  0, vxdID); // restrain hits to sensor boundaries

  return { localUPosition, localVPosition };
}


B2Vector3<double> DATCONSVDSpacePoint::getGlobalCoordinates(std::pair<double, double> const& hitLocal, VxdID vxdID,
                                                            VXD::SensorInfoBase const* aSensorInfo)
{
  //We need some handle to translate IDs to local and global coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  return aSensorInfo->pointToGlobal(TVector3(hitLocal.first, hitLocal.second, 0));
}
