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
#include <svd/dataobjects/SVDCluster.h>

using namespace Belle2;

CKFToSVDResult::CKFToSVDResult(const std::vector<TrackFindingCDC::WithWeight<const CKFToSVDState*>>& path) :
  Super(path, path.back()->getMeasuredStateOnPlane())
{
  B2ASSERT("Path should not be empty", not path.empty());

  for (const TrackFindingCDC::WithWeight<const CKFToSVDState*>& state : path) {
    const RecoTrack* relatedSVDTrack = state->getRelatedSVDTrack();
    if (m_relatedSVDRecoTrack) {
      B2ASSERT("There is a state with a different VXD track in it!", m_relatedSVDRecoTrack == relatedSVDTrack);
    } else {
      m_relatedSVDRecoTrack = relatedSVDTrack;
    }
  }
}

void CKFToSVDResult::addToRecoTrack(RecoTrack& recoTrack) const
{
  unsigned int sortingParameter = 0;
  for (const SpacePoint* spacePoint : TrackFindingCDC::reversedRange(getHits())) {
    RelationVector<SVDCluster> relatedClusters = spacePoint->getRelationsTo<SVDCluster>();
    for (const SVDCluster& cluster : relatedClusters) {
      recoTrack.addSVDHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_VXDTrackFinder);
      sortingParameter++;
    }
  }
}

const RecoTrack* CKFToSVDResult::getRelatedSVDRecoTrack() const
{
  return m_relatedSVDRecoTrack;
}
