/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/SeededSVDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <svd/dataobjects/SVDCluster.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
SeededSVDPairFilter::operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation)
{
  const CKFToSVDState& fromState = *(relation.first);
  const SpacePoint* fromSpacePoint = fromState.getHit();

  if (not fromSpacePoint) {
    return 1.0;
  }

  std::map<const RecoTrack*, unsigned int> relatedRecoTrackHistogram;

  for (const CKFToSVDState* state : {relation.first, relation.second}) {
    const SpacePoint* spacePoint = state->getHit();
    for (const SVDCluster& cluster : spacePoint->getRelationsTo<SVDCluster>()) {
      const auto& relatedRecoTracks = cluster.getRelationsTo<RecoTrack>(m_param_vxdTracksStoreArrayName);
      if (relatedRecoTracks.size() == 0) {
        // if we have one cluster, that is not connected to any track, we do not want to have it
        return NAN;
      }
      for (const RecoTrack& recoTrack : relatedRecoTracks) {
        const RecoTrack* recoTrackPointer = &recoTrack;
        if (relatedRecoTrackHistogram.find(recoTrackPointer) == relatedRecoTrackHistogram.end()) {
          relatedRecoTrackHistogram[recoTrackPointer] = 1;
        } else {
          relatedRecoTrackHistogram[recoTrackPointer]++;
        }
      }
    }
  }

  const auto maximalElement = std::max_element(relatedRecoTrackHistogram.begin(), relatedRecoTrackHistogram.end(),
                                               TrackFindingCDC::LessOf<TrackFindingCDC::Second>());

  if (maximalElement->second >= 4 - m_param_maximalNumberOfDifferentRecoTracks) {
    return 1;
  }

  return NAN;
}

void SeededSVDPairFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vxdTracksStoreArrayName"), m_param_vxdTracksStoreArrayName,
                                "Store Array name coming from VXDTF2",
                                m_param_vxdTracksStoreArrayName);
}