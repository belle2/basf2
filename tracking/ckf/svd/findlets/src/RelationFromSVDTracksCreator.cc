/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/svd/findlets/RelationFromSVDTracksCreator.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/ReversedRange.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

RelationFromSVDTracksCreator::RelationFromSVDTracksCreator()
{
}

void RelationFromSVDTracksCreator::initialize()
{
  Super::initialize();
  m_vxdRecoTracks.isRequired(m_param_vxdTracksStoreArrayName);
}

RelationFromSVDTracksCreator::~RelationFromSVDTracksCreator() = default;

void RelationFromSVDTracksCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vxdTracksStoreArrayName"),
                                m_param_vxdTracksStoreArrayName,
                                "Store Array name for tracks coming from VXDTF2.");
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cdcTracksStoreArrayName"),
                                m_param_cdcTracksStoreArrayName,
                                "Store Array name for tracks coming from CDCTF.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "spacePointTrackCandidatesName"),
                                m_param_spacePointTrackCandidateName,
                                "Store Array name for the SpacePointTrackCandidates coming from VXDTF2.",
                                m_param_spacePointTrackCandidateName);
}

void RelationFromSVDTracksCreator::apply(std::vector<CKFToSVDState>& seedStates, std::vector<CKFToSVDState>& states,
                                         std::vector<TrackFindingCDC::WeightedRelation<CKFToSVDState>>& relations)
{
  for (const RecoTrack& vxdRecoTrack : m_vxdRecoTracks) {
    if (vxdRecoTrack.getRelated<RecoTrack>(m_param_cdcTracksStoreArrayName)) {
      continue;
    }

    CKFToSVDState* currentState = nullptr;

    const SpacePointTrackCand* spacePointTrackCand =
      vxdRecoTrack.getRelated<SpacePointTrackCand>(m_param_spacePointTrackCandidateName);

    B2ASSERT("There should be a related SPTC!", spacePointTrackCand);
    const std::vector<const SpacePoint*> spacePoints = spacePointTrackCand->getSortedHits();

    for (const SpacePoint* spacePoint : TrackFindingCDC::reversedRange(spacePoints)) {
      const auto hasSpacePoint = [spacePoint](const CKFToSVDState & state) {
        return state.getHit() == spacePoint;
      };

      const auto nextStateIterator = std::find_if(states.begin(), states.end(), hasSpacePoint);
      B2ASSERT("State can not be none!", nextStateIterator != states.end());

      CKFToSVDState& nextState = *nextStateIterator;
      nextState.setRelatedSVDTrack(&vxdRecoTrack);

      if (currentState) {
        relations.emplace_back(currentState, NAN, &nextState);
      } else {
        for (CKFToSVDState& seedState : seedStates) {
          // We are not setting the related SVD track of the first state!
          relations.emplace_back(&seedState, NAN, &nextState);
        }
      }

      currentState = &nextState;
    }
  }

  std::sort(relations.begin(), relations.end());
}
