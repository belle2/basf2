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
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

using namespace std;
using namespace Belle2;

ClassImp(SpacePoint)

SpacePoint::SpacePoint(const PXDCluster* pxdCluster,
                       unsigned int indexNumber,
                       unsigned short nameIndex,
                       const VXD::SensorInfoBase* aSensorInfo) :
  m_vxdID(pxdCluster->getSensorID()),
  m_nameIndex(nameIndex)
{
  if (pxdCluster == NULL) { throw InvalidNumberOfClusters(); }
  m_indexNumbers.push_back(indexNumber);

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  m_position = aSensorInfo->pointToGlobal(
                 TVector3(
                   pxdCluster->getU(),
                   pxdCluster->getV(),
                   0
                 )
               );

  setPositionError(pxdCluster->getUSigma(), pxdCluster->getVSigma(), aSensorInfo);

  m_normalizedLocal = convertLocalToNormalizedCoordinates({ pxdCluster->getU(), pxdCluster->getV() } , m_vxdID, aSensorInfo);

  m_sensorType = aSensorInfo->getType();
}



SpacePoint::SpacePoint(const std::vector<SpacePoint::SVDClusterInformation>& clusters,
                       unsigned short nameIndex,
                       const VXD::SensorInfoBase* aSensorInfo) :
  m_nameIndex(nameIndex)
{
  unsigned int nClusters = clusters.size();
  double uCoord = 0; // 0 = center of Sensor
  double vCoord = 0; // 0 = center of Sensor
  double uSigma = -1; // negative sigmas are not possible, setting to -1 for catching cases of missing Cluster
  double vSigma = -1; // negative sigmas are not possible, setting to -1 for catching cases of missing Cluster

  // do checks for sanity of input:
  if (nClusters == 0 or nClusters > 2) {
    throw InvalidNumberOfClusters();
  } else {
    vector<VxdID::baseType> vxdIDs;
    vector<bool> isUType;
    for (vector<SVDClusterInformation>::const_iterator iter = clusters.begin(); iter < clusters.end(); ++iter) {
      if (iter->first == NULL) throw InvalidNumberOfClusters();
      vxdIDs.push_back(iter->first->getSensorID());
      isUType.push_back(iter->first->isUCluster());
      m_indexNumbers.push_back(iter->second);
    }

    auto newEndVxdID = std::unique(vxdIDs.begin(), vxdIDs.end());
    vxdIDs.resize(std::distance(vxdIDs.begin(), newEndVxdID));

    auto newEndUType = std::unique(isUType.begin(), isUType.end());
    isUType.resize(std::distance(isUType.begin(), newEndUType));

    if (vxdIDs.size() != 1 or isUType.size() != nClusters) throw IncompatibleClusters();
  }

  m_vxdID = clusters[0].first->getSensorID();

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  // retrieve position and sigma-values
  for (vector<SVDClusterInformation>::const_iterator iter = clusters.begin(); iter < clusters.end(); ++iter) {
    if (iter->first->isUCluster() == true) {
      uCoord = iter->first->getPosition();
      uSigma = iter->first->getPositionSigma();
    } else {
      vCoord = iter->first->getPosition();
      vSigma = iter->first->getPositionSigma();
    }
  }

  m_position = aSensorInfo->pointToGlobal(
                 TVector3(
                   uCoord,
                   vCoord,
                   0
                 )
               );

  // if sigma for a coordinate is not known, a uniform distribution over the whole sensor is asumed:
  if (uSigma < 0) { uSigma = aSensorInfo->getUSize(vCoord) / sqrt(12); }
  if (vSigma < 0) { vSigma = aSensorInfo->getVSize() / sqrt(12); }

  setPositionError(uSigma, vSigma, aSensorInfo);

  m_normalizedLocal = convertLocalToNormalizedCoordinates({ uCoord, vCoord } , m_vxdID, aSensorInfo);

  m_sensorType = aSensorInfo->getType();
}



vector< genfit::PlanarMeasurement > SpacePoint::getGenfitCompatible() const
{
  vector< genfit::PlanarMeasurement > collectedMeasurements;

  const StoreObjPtr<SpacePointMetaInfo> metaInfo;

  // get the related clusters to this spacePoint and create a genfit::PlanarMeasurement for each of them:
  if (getType() == VXD::SensorInfoBase::SensorType::SVD) {

    auto relatedClusters = this->getRelationsTo<SVDCluster>(metaInfo->getName(m_nameIndex));
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(SVDRecoHit(relatedClusters[i]));
    }

  } else if (getType() == VXD::SensorInfoBase::SensorType::PXD) {

    collectedMeasurements.push_back(
      PXDRecoHit(this->getRelatedTo<PXDCluster>(metaInfo->getName(m_nameIndex)))
    );

  } else {
    throw InvalidDetectorType();
  }


  return move(collectedMeasurements);
}



std::pair<double, double> SpacePoint::convertLocalToNormalizedCoordinates(const std::pair<double, double>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
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

  double localUPosition = hitLocal.first +  0.5 * sensorSizeU;
  localUPosition /= sensorSizeU;
  boundaryCheck(localUPosition, 0, 1);
  double localVPosition = hitLocal.second +  0.5 * sensorSizeV;
  localVPosition /= sensorSizeV;
  boundaryCheck(localVPosition, 0, 1);

  return { localUPosition, localVPosition };
}



// std::pair<double, double> SpacePoint::convertToLocalCoordinates(const std::pair<double, double>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
// {
//   //We need some handle to translate IDs to local and global
//   // coordinates.
//   if (aSensorInfo == NULL) {
//     aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
//   }
//
//   // Changed by Stefan F
//   double localUPosition = hitNormalized.first - (0.5 * aSensorInfo->getUSize());
//   double localVPosition = hitNormalized.second - (0.5 * aSensorInfo->getVSize());
//
//   // old ones:
//   //   double localVPosition = (hitNormalized.second - 0.5) * aSensorInfo->getVSize();
//   //   double localUPosition = (hitNormalized.first - 0.5) * aSensorInfo->getUSize();
//
//   return (make_pair(localUPosition, localVPosition));
// }



std::pair<double, double> SpacePoint::convertNormalizedToLocalCoordinates(const std::pair<double, double>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  // normalized range is 0 to 1, but final coordinates are from - halfSensorSize to + halfSensorSize
  double localVPosition = (hitNormalized.second - 0.5) * aSensorInfo->getVSize();
  boundaryCheck(localVPosition, -0.5 * aSensorInfo->getVSize(), 0.5 * aSensorInfo->getVSize()); // restrain hits to sensor boundaries

  double uSizeAtHit = aSensorInfo->getUSize(localVPosition);
  double localUPosition = (hitNormalized.first - 0.5) * uSizeAtHit;
  boundaryCheck(localUPosition, -0.5 * aSensorInfo->getUSize(), uSizeAtHit); // restrain hits to sensor boundaries

  return { localUPosition, localVPosition };
}




TVector3 SpacePoint::getGlobalCoordinates(const std::pair<double, double>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  return move(aSensorInfo->pointToGlobal(
                TVector3(
                  hitLocal.first,
                  hitLocal.second,
                  0
                )
              )
             );
}



