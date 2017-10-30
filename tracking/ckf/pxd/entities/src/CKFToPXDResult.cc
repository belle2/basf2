/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>

#include <tracking/trackFindingCDC/utilities/ReversedRange.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDCluster.h>

using namespace Belle2;

CKFToPXDResult::CKFToPXDResult(const std::vector<TrackFindingCDC::WithWeight<const CKFToPXDState*>>& path) :
  Super(path, path.back()->getMeasuredStateOnPlane())
{
}

void CKFToPXDResult::addToRecoTrack(RecoTrack& recoTrack) const
{
  for (const SpacePoint* spacePoint : TrackFindingCDC::reversedRange(getHits())) {
    unsigned int sortingParameter = 0;

    RelationVector<PXDCluster> relatedClusters = spacePoint->getRelationsTo<PXDCluster>();
    for (const PXDCluster& cluster : relatedClusters) {
      recoTrack.addPXDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
      sortingParameter++;
    }
  }
}