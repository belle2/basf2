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
  addProcessingSignalListener(&m_relationCreator);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_overlapFilter);
  addProcessingSignalListener(&m_bestMatchSelector);
}

void CKFToSVDSeedFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_dataHandler.exposeParameters(moduleParamList, prefix);
  m_hitsLoader.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromTracks.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromHits.exposeParameters(moduleParamList, prefix);
  m_relationCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_bestMatchSelector.exposeParameters(moduleParamList, prefix);
  m_overlapFilter.exposeParameters(moduleParamList, prefix);

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

  m_relationsCDCToSVD.clear();
}

void CKFToSVDSeedFindlet::apply()
{
  m_dataHandler.apply(m_cdcRecoTrackVector);
  m_hitsLoader.apply(m_spacePointVector);

  m_stateCreatorFromTracks.apply(m_cdcRecoTrackVector, m_seedStates);
  m_stateCreatorFromHits.apply(m_spacePointVector, m_states);

  m_relationCreator.apply(m_seedStates, m_states, m_relations);
  B2DEBUG(50, "Created " << m_relations.size() << " relations.");

  m_treeSearchFindlet.apply(m_seedStates, m_states, m_relations, m_results);
  B2DEBUG(50, "Having found " << m_results.size() << " results before overlap check");

  const auto hasLowHitNumber = [this](const CKFResult<RecoTrack, SpacePoint>& result) {
    return result.getHits().size() < m_param_minimalHitRequirement;
  };
  TrackFindingCDC::erase_remove_if(m_results, hasLowHitNumber);
  B2DEBUG(50, "After filtering: Having found " << m_results.size() << " results before overlap check");


  // TODO: in findlet
  for (const CKFToSVDResult& result : m_results) {
    const Weight weight = m_overlapFilter(result);
    if (not std::isnan(weight)) {
      const RecoTrack* relatedSVDTrack = result.getRelatedSVDRecoTrack();
      m_relationsCDCToSVD.emplace_back(result.getSeed(), weight, relatedSVDTrack);
    }
  }

  m_bestMatchSelector.apply(m_relationsCDCToSVD);

  // TODO: in findlet
  // write out relations
  for (const TrackFindingCDC::WeightedRelation<const RecoTrack, const RecoTrack>& relation : m_relationsCDCToSVD) {
    const RecoTrack* cdcTrack = relation.getFrom();
    const RecoTrack* svdTrack = relation.getTo();
    cdcTrack->addRelationTo(svdTrack);
  }
}
