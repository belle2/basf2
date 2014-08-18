/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <testbeam/vxd/tracking/spacePointCreation/TBSpacePoint.h>
#include <vxd/dataobjects/VxdID.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <testbeam/vxd/reconstruction/TelRecoHit.h>

using namespace std;
using namespace Belle2;

ClassImp(TBSpacePoint)

SpacePointMetaInfo SpacePoint::m_metaInfo = SpacePointMetaInfo();

TBSpacePoint::TBSpacePoint(const TelCluster* telCluster,
                           unsigned int indexNumber,
                           unsigned short nameIndex,
                           const VXD::SensorInfoBase* aSensorInfo)
{
  SpacePoint::m_nameIndex = nameIndex;
  SpacePoint::m_vxdID = telCluster->getSensorID();
  if (telCluster == NULL) { throw SpacePoint::InvalidNumberOfClusters(); }
  SpacePoint::m_indexNumbers.push_back(indexNumber);

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(SpacePoint::m_vxdID);
  }

  SpacePoint::m_position = aSensorInfo->pointToGlobal(
                             TVector3(
                               telCluster->getU(),
                               telCluster->getV(),
                               0
                             )
                           );

  SpacePoint::setPositionError(telCluster->getUSigma(), telCluster->getVSigma(), aSensorInfo);

  SpacePoint::m_normalizedLocal = SpacePoint::convertLocalToNormalizedCoordinates({ telCluster->getU(), telCluster->getV() } , SpacePoint::m_vxdID, aSensorInfo);

  SpacePoint::m_sensorType = aSensorInfo->getType();
}



vector< genfit::PlanarMeasurement > TBSpacePoint::getGenfitCompatible()
{
  vector< genfit::PlanarMeasurement > collectedMeasurements;


  // get the related clusters to this spacePoint and create a genfit::PlanarMeasurement for each of them:
  if (getType() == VXD::SensorInfoBase::SensorType::SVD) {

    auto relatedClusters = this->getRelationsTo<SVDCluster>(SpacePoint::getClusterStoreName());
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(SVDRecoHit(relatedClusters[i]));
    }

  } else if (getType() == VXD::SensorInfoBase::SensorType::PXD) {

    collectedMeasurements.push_back(
      PXDRecoHit(this->getRelatedTo<PXDCluster>(SpacePoint::getClusterStoreName()))
    );

  } else if (getType() == VXD::SensorInfoBase::SensorType::TEL) {

    collectedMeasurements.push_back(
      TelRecoHit(this->getRelatedTo<TelCluster>(SpacePoint::getClusterStoreName()))
    );

  } else {
    throw SpacePoint::InvalidDetectorType();
  }


  return move(collectedMeasurements);
}
