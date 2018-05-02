/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

using namespace std;
using namespace Belle2;

//---PXD related constructor---
SpacePoint::SpacePoint(const PXDCluster* pxdCluster,
                       const VXD::SensorInfoBase* aSensorInfo) :  m_clustersAssigned( {true, true}), m_vxdID(pxdCluster->getSensorID())
{
  //We need some handle to translate IDs to local and global coordinates.
  //aSensorInfo exists only for testing purposes, so this is the relevant case!
  if (aSensorInfo == nullptr) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  // the second parameter set to true results in alignment constants being applied
  m_position = aSensorInfo->pointToGlobal(TVector3(pxdCluster->getU(), pxdCluster->getV(), 0), true);

  setPositionError(pxdCluster->getUSigma(), pxdCluster->getVSigma(), aSensorInfo);

  m_normalizedLocal = convertLocalToNormalizedCoordinates({ pxdCluster->getU(), pxdCluster->getV() },
                                                          m_vxdID, aSensorInfo);

  m_sensorType = aSensorInfo->getType();
}


//---SVD related constructor---
SpacePoint::SpacePoint(std::vector<const SVDCluster*>& clusters,
                       const VXD::SensorInfoBase* aSensorInfo)
{
  //---The following contains only sanity checks without effect, if nobody gave buggy information---
  //We have 1 or two SVD Clusters.
  B2ASSERT("You have to insert 1 or two SVD Clusters, but gave: " << clusters.size(), ((clusters.size() == 1)
           || (clusters.size() == 2)));

  //No cluster pointer is a nullptr.
  for (auto && cluster : clusters) {
    B2ASSERT("An SVDCluster Pointer is a nullptr!", cluster != nullptr);
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
  double uSigma = -1; // negative sigmas are not possible, setting to -1 for catching cases of missing Cluster
  double vSigma = -1; // negative sigmas are not possible, setting to -1 for catching cases of missing Cluster

  const SVDCluster* vCluster(NULL), *uCluster(NULL);
  for (const SVDCluster* aCluster : clusters) {
    if (aCluster->isUCluster() == true) {
      m_clustersAssigned.first = true;
      uCoord = aCluster->getPosition();
      uSigma = aCluster->getPositionSigma();
      uCluster = aCluster;
    } else {
      m_clustersAssigned.second = true;
      vCoord = aCluster->getPosition();
      vSigma = aCluster->getPositionSigma();
      vCluster = aCluster;
    }
  }

  if (aSensorInfo->getBackwardWidth() > aSensorInfo->getForwardWidth() &&
      vCluster != NULL && uCluster != NULL) // is a WedgeSensor and we do have a vCluster
    uCoord = uCluster->getPosition(vCoord);

  // the second parameter set to true results in alignment constants being applied
  m_position = aSensorInfo->pointToGlobal(TVector3(uCoord, vCoord, 0), true);
  m_normalizedLocal = convertLocalToNormalizedCoordinates({ uCoord, vCoord } , m_vxdID, aSensorInfo);

  // if sigma for a coordinate is not known, a uniform distribution over the whole sensor is asumed:
  if (uSigma < 0) {
    uSigma = aSensorInfo->getUSize(vCoord) / sqrt(12.);
  }
  if (vSigma < 0) {
    vSigma = aSensorInfo->getVSize() / sqrt(12.);
  }

  setPositionError(uSigma, vSigma, aSensorInfo);

  //retrieve and set hit times
  for (const SVDCluster* aCluster : clusters)
    if (aCluster->isUCluster() == true)
      m_UClusterTime = aCluster->getClsTime();
    else
      m_VClusterTime = aCluster->getClsTime();

}


vector< genfit::PlanarMeasurement > SpacePoint::getGenfitCompatible() const
{
  // XYRecoHit will be stored as their base-class, which is detector-independent.
  vector< genfit::PlanarMeasurement > collectedMeasurements;


  // get the related clusters to this spacePoint and create a genfit::PlanarMeasurement for each of them:
  if (getType() == VXD::SensorInfoBase::SensorType::SVD) {

    auto relatedClusters = this->getRelationsTo<SVDCluster>("ALL");
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(SVDRecoHit(relatedClusters[i]));
    }

  } else if (getType() == VXD::SensorInfoBase::SensorType::PXD) {

    // since we do not know the name of the attached PXDCluster, getRelatedTo does not work, however, getRelationsTo seems to be less sensible and therefore can be used, but in this case, one has to loop over the entries (which should be only one in this case)
    auto relatedClusters = this->getRelationsTo<PXDCluster>("ALL");
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(PXDRecoHit(relatedClusters[i]));
    }

  } else {
    B2FATAL("unknown detector type");
  }

  B2DEBUG(50, "SpacePoint::getGenfitCompatible(): collected " << collectedMeasurements.size() << " meaturements");

  return collectedMeasurements;
}


std::pair<double, double> SpacePoint::convertLocalToNormalizedCoordinates(
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



std::pair<double, double> SpacePoint::convertNormalizedToLocalCoordinates(
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


B2Vector3<double> SpacePoint::getGlobalCoordinates(std::pair<double, double> const& hitLocal, VxdID vxdID,
                                                   VXD::SensorInfoBase const* aSensorInfo)
{
  //We need some handle to translate IDs to local and global coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  // the second parameter set to true results in alignment constants being applied
  return aSensorInfo->pointToGlobal(TVector3(hitLocal.first, hitLocal.second, 0), true);
}
