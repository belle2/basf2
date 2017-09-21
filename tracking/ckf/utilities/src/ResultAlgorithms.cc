/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/utilities/ResultAlgorithms.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <tracking/dataobjects/RecoHitInformation.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>

using namespace Belle2;

template <>
void RecoTrackHitsAdder::addHitToRecoTrack(const SpacePoint* const& spacePoint, RecoTrack& newRecoTrack,
                                           unsigned int& sortingParameter)
{
  const int detID = spacePoint->getType();

  if (detID == VXD::SensorInfoBase::PXD) {
    RelationVector<PXDCluster> relatedClusters = spacePoint->getRelationsTo<PXDCluster>();
    for (const PXDCluster& cluster : relatedClusters) {
      newRecoTrack.addPXDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
      sortingParameter++;
    }
  } else if (detID == VXD::SensorInfoBase::SVD) {
    RelationVector<SVDCluster> relatedClusters = spacePoint->getRelationsTo<SVDCluster>();
    for (const SVDCluster& cluster : relatedClusters) {
      newRecoTrack.addSVDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
      sortingParameter++;
    }
  }
}