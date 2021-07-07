/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/ckf/vtx/findlets/RelationFromVTXTracksCreator.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/ReversedRange.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;

RelationFromVTXTracksCreator::RelationFromVTXTracksCreator()
{
}

void RelationFromVTXTracksCreator::initialize()
{
  Super::initialize();
  m_vxdRecoTracks.isRequired(m_param_vtxTracksStoreArrayName);
}

RelationFromVTXTracksCreator::~RelationFromVTXTracksCreator() = default;

void RelationFromVTXTracksCreator::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vtxTracksStoreArrayName"),
                                m_param_vtxTracksStoreArrayName,
                                "Store Array name for tracks coming from VXDTF2.");
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cdcTracksStoreArrayName"),
                                m_param_cdcTracksStoreArrayName,
                                "Store Array name for tracks coming from CDCTF.");

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "spacePointTrackCandidatesName"),
                                m_param_spacePointTrackCandidateName,
                                "Store Array name for the SpacePointTrackCandidates coming from VXDTF2.",
                                m_param_spacePointTrackCandidateName);
}

void RelationFromVTXTracksCreator::apply(std::vector<CKFToVTXState>& seedStates, std::vector<CKFToVTXState>& states,
                                         std::vector<TrackFindingCDC::WeightedRelation<CKFToVTXState>>& relations)
{
  for (const RecoTrack& vxdRecoTrack : m_vxdRecoTracks) {
    if (vxdRecoTrack.getRelated<RecoTrack>(m_param_cdcTracksStoreArrayName)) {
      continue;
    }

    CKFToVTXState* currentState = nullptr;

    const SpacePointTrackCand* spacePointTrackCand =
      vxdRecoTrack.getRelated<SpacePointTrackCand>(m_param_spacePointTrackCandidateName);

    B2ASSERT("There should be a related SPTC!", spacePointTrackCand);
    const std::vector<const SpacePoint*> spacePoints = spacePointTrackCand->getSortedHits();

    for (const SpacePoint* spacePoint : TrackFindingCDC::reversedRange(spacePoints)) {
      const auto hasSpacePoint = [spacePoint](const CKFToVTXState & state) {
        return state.getHit() == spacePoint;
      };

      const auto nextStateIterator = std::find_if(states.begin(), states.end(), hasSpacePoint);
      B2ASSERT("State can not be none!", nextStateIterator != states.end());

      CKFToVTXState& nextState = *nextStateIterator;
      nextState.setRelatedVTXTrack(&vxdRecoTrack);

      if (currentState) {
        relations.emplace_back(currentState, NAN, &nextState);
      } else {
        for (CKFToVTXState& seedState : seedStates) {
          // We are not setting the related VTX track of the first state!
          // cppcheck-suppress useStlAlgorithm
          relations.emplace_back(&seedState, NAN, &nextState);
        }
      }

      currentState = &nextState;
    }
  }

  std::sort(relations.begin(), relations.end());
}
