/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/vtx/entities/CKFToVTXResult.h>

#include <tracking/trackFindingCDC/utilities/ReversedRange.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vtx/dataobjects/VTXCluster.h>

using namespace Belle2;

CKFToVTXResult::CKFToVTXResult(const std::vector<TrackFindingCDC::WithWeight<const CKFToVTXState*>>& path) :
  Super(path, path.back()->getMeasuredStateOnPlane())
{
  B2ASSERT("Path should not be empty", not path.empty());

  for (const TrackFindingCDC::WithWeight<const CKFToVTXState*>& state : path) {
    const RecoTrack* relatedVTXTrack = state->getRelatedVTXTrack();
    if (m_relatedVTXRecoTrack) {
      B2ASSERT("There is a state with a different VTX track in it!", m_relatedVTXRecoTrack == relatedVTXTrack);
    } else {
      m_relatedVTXRecoTrack = relatedVTXTrack;
    }
  }
}

void CKFToVTXResult::addToRecoTrack(RecoTrack& recoTrack) const
{
  unsigned int sortingParameter = 0;
  for (const SpacePoint* spacePoint : TrackFindingCDC::reversedRange(getHits())) {
    RelationVector<VTXCluster> relatedClusters = spacePoint->getRelationsTo<VTXCluster>();
    for (const VTXCluster& cluster : relatedClusters) {
      recoTrack.addVTXHit(&cluster, sortingParameter, Belle2::RecoHitInformation::c_CDCtoVTXCKF);
      sortingParameter++;
    }
  }
}

const RecoTrack* CKFToVTXResult::getRelatedVTXRecoTrack() const
{
  return m_relatedVTXRecoTrack;
}
