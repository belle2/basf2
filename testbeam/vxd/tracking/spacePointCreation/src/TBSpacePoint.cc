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

TBSpacePoint::TBSpacePoint(const TelCluster* telCluster,
                           const VXD::SensorInfoBase* aSensorInfo)
{
  SpacePoint::m_clustersAssigned = {true, true};
  SpacePoint::m_vxdID = telCluster->getSensorID();
  B2ASSERT("Creation of TBSpacePoint from nullptr not possible", telCluster != nullptr);
  SpacePoint::m_qualityIndicator = 0.5;
  SpacePoint::m_isAssigned = false;

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

  SpacePoint::m_normalizedLocal = SpacePoint::convertLocalToNormalizedCoordinates({ telCluster->getU(), telCluster->getV() } ,
                                  SpacePoint::m_vxdID, aSensorInfo);

  SpacePoint::m_sensorType = aSensorInfo->getType();
}



vector< genfit::PlanarMeasurement > TBSpacePoint::getGenfitCompatible() const
{
  // XYRecoHit will be stored as their base-class, which is detector-independent.
  vector< genfit::PlanarMeasurement > collectedMeasurements;

  // get the related clusters to this spacePoint and create a genfit::PlanarMeasurement for each of them:
  if (getType() == VXD::SensorInfoBase::SensorType::TEL) {

    // since we do not know the name of the attached PXDCluster, getRelatedTo does not work, however, getRelationsTo seems to be less sensible and therefore can be used, but in this case, one has to loop over the entries (which should be only one in this case)
    auto relatedClusters = this->getRelationsTo<TelCluster>("ALL");
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(TelRecoHit(relatedClusters[i]));
    }

  } else if (getType() == VXD::SensorInfoBase::SensorType::SVD) {

    auto relatedClusters = this->getRelationsTo<SVDCluster>("ALL");
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(SVDRecoHit(relatedClusters[i]));
    }

  } else if (getType() == VXD::SensorInfoBase::SensorType::PXD) {

    // same issue as TelClusters
    auto relatedClusters = this->getRelationsTo<PXDCluster>("ALL");
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(PXDRecoHit(relatedClusters[i]));
    }

  } else {
    //TODO: understand if this check is necessary and perform it, but this exception is deleted from the tracking package.
    //throw SpacePoint::InvalidDetectorType();
  }

  B2DEBUG(50, "TBSpacePoint::getGenfitCompatible(): collected " << collectedMeasurements.size() << " meaturements");

  return collectedMeasurements;
}
