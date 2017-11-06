/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/findlets/CKFToSVDSeedFindlet.h>

#include <tracking/ckf/general/findlets/CKFDataHandler.icc.h>
#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>
#include <tracking/ckf/general/findlets/OnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LimitedOnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LayerToggledApplier.icc.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/trackFindingCDC/utilities/ReversedRange.h>
#include <tracking/trackFindingCDC/collectors/selectors/BestMatchSelector.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <framework/core/ModuleParamList.dcl.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CKFToSVDSeedFindlet::~CKFToSVDSeedFindlet() = default;

CKFToSVDSeedFindlet::CKFToSVDSeedFindlet()
{
  addProcessingSignalListener(&m_dataHandler);
  addProcessingSignalListener(&m_hitsLoader);
  addProcessingSignalListener(&m_stateCreatorFromTracks);
  addProcessingSignalListener(&m_stateCreatorFromHits);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_overlapFilter);
}

void CKFToSVDSeedFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_dataHandler.exposeParameters(moduleParamList, prefix);
  m_hitsLoader.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromTracks.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromHits.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_overlapFilter.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "vxdTracksStoreArrayName"), m_param_vxdTracksStoreArrayName,
                                "Store Array name coming from VXDTF2.", m_param_vxdTracksStoreArrayName);

  moduleParamList->addParameter("minimalHitRequirement", m_param_minimalHitRequirement,
                                "Minimal Hit requirement for the results (counted in space points)",
                                m_param_minimalHitRequirement);
}

void CKFToSVDSeedFindlet::beginEvent()
{
  Super::beginEvent();

  m_cdcRecoTrackVector.clear();
  m_spacePointVector.clear();

  m_seedStates.clear();
  m_states.clear();
  m_relations.clear();

  m_results.clear();
}

void CKFToSVDSeedFindlet::apply()
{
  m_dataHandler.apply(m_cdcRecoTrackVector);
  m_hitsLoader.apply(m_spacePointVector);

  m_stateCreatorFromTracks.apply(m_cdcRecoTrackVector, m_seedStates);
  m_stateCreatorFromHits.apply(m_spacePointVector, m_states);

  StoreArray<RecoTrack> vxdRecoTracks(m_param_vxdTracksStoreArrayName);

  for (const RecoTrack& vxdRecoTrack : vxdRecoTracks) {
    CKFToSVDState* currentState = nullptr;

    // TODO: can this be done better?
    const SpacePointTrackCand* spacePointTrackCand = vxdRecoTrack.getRelated<SpacePointTrackCand>("SPTrackCands");

    B2ASSERT("There should be a related SPTC!", spacePointTrackCand);
    const std::vector<const SpacePoint*> spacePoints = spacePointTrackCand->getSortedHits();
    for (const SpacePoint* spacePoint : TrackFindingCDC::reversedRange(spacePoints)) {

      CKFToSVDState* nextState = nullptr;
      for (CKFToSVDState& state : m_states) {
        if (state.getHit() == spacePoint) {
          nextState = &state;
        }
      }

      B2ASSERT("State can not be none!", nextState);

      nextState->setRelatedSVDTrack(&vxdRecoTrack);

      if (currentState) {
        m_relations.emplace_back(currentState, NAN, nextState);
      } else {
        for (CKFToSVDState& seedState : m_seedStates) {
          // We are not setting the related SVD track of the first state!
          m_relations.emplace_back(&seedState, NAN, nextState);
        }
      }

      currentState = nextState;
    }
  }

  std::sort(m_relations.begin(), m_relations.end());

  B2DEBUG(50, "Created " << m_relations.size() << " relations.");

  m_treeSearchFindlet.apply(m_seedStates, m_states, m_relations, m_results);
  B2DEBUG(50, "Having found " << m_results.size() << " results before overlap check");

  const auto hasLowHitNumber = [this](const CKFResult<RecoTrack, SpacePoint>& result) {
    return result.getHits().size() < m_param_minimalHitRequirement;
  };
  TrackFindingCDC::erase_remove_if(m_results, hasLowHitNumber);
  B2DEBUG(50, "After filtering: Having found " << m_results.size() << " results before overlap check");


  std::vector<TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>> relatedCDCToSVD;
  for (const CKFToSVDResult& result : m_results) {
    const RecoTrack* relatedSVDTrack = result.getRelatedSVDRecoTrack();
    const Weight weight = m_overlapFilter(result);
    if (not std::isnan(weight)) {
      relatedCDCToSVD.emplace_back(result.getSeed(), weight, relatedSVDTrack);
    }
  }

  std::sort(relatedCDCToSVD.begin(), relatedCDCToSVD.end(), TrackFindingCDC::GreaterWeight());

  // sort out
  TrackFindingCDC::BestMatchSelector<const RecoTrack, const RecoTrack> bestMatchSelector;
  bestMatchSelector.apply(relatedCDCToSVD);

  // write out relations
  for (const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>& relation : relatedCDCToSVD) {
    const RecoTrack* cdcTrack = relation.getFrom();
    const RecoTrack* svdTrack = relation.getTo();
    cdcTrack->addRelationTo(svdTrack);
  }
}
