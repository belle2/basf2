/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/entities/CKFToSVDResult.h>

#include <tracking/trackFindingCDC/utilities/ReversedRange.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDCluster.h>

using namespace Belle2;

CKFToSVDResult::CKFToSVDResult(const std::vector<const CKFToSVDState*>& path) :
  Super(path, path.back()->getMeasuredStateOnPlane())
{
}

void CKFToSVDResult::addToRecoTrack(RecoTrack& recoTrack) const
{
  for (const SpacePoint* spacePoint : TrackFindingCDC::reversedRange(getHits())) {
    const int detID = spacePoint->getType();
    unsigned int sortingParameter = 0;

    if (detID == VXD::SensorInfoBase::PXD) {
      RelationVector<PXDCluster> relatedClusters = spacePoint->getRelationsTo<PXDCluster>();
      for (const PXDCluster& cluster : relatedClusters) {
        recoTrack.addPXDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
        sortingParameter++;
      }
    } else if (detID == VXD::SensorInfoBase::SVD) {
      RelationVector<SVDCluster> relatedClusters = spacePoint->getRelationsTo<SVDCluster>();
      for (const SVDCluster& cluster : relatedClusters) {
        recoTrack.addSVDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
        sortingParameter++;
      }
    }
  }
}