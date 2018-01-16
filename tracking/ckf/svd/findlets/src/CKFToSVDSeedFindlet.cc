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

#include <tracking/ckf/general/findlets/SpacePointTagger.icc.h>
#include <tracking/ckf/general/findlets/CKFDataHandler.icc.h>
#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/CKFRelationCreator.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/OverlapResolver.icc.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>
#include <tracking/ckf/general/findlets/OnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LimitedOnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LayerToggledApplier.icc.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/ChoosableFromVarSetFilter.icc.h>
#include <tracking/ckf/svd/filters/relations/LayerSVDRelationFilter.icc.h>

#include <framework/core/ModuleParamList.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>

#include <tracking/spacePointCreation/SpacePointTrackCand.h>

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
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_unusedTracksAdder);
  addProcessingSignalListener(&m_spacePointTagger);
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
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
  m_unusedTracksAdder.exposeParameters(moduleParamList, prefix);
  m_spacePointTagger.exposeParameters(moduleParamList, prefix);
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
  m_filteredResults.clear();
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

  TrackFindingCDC::erase_remove_if(m_results, [](const auto & result) {
    return result.getHits().empty();
  });

  m_overlapResolver.apply(m_results, m_filteredResults);

  m_unusedTracksAdder.apply(m_filteredResults);
  m_dataHandler.store(m_filteredResults);

  // Reassign the space points according to the new results
  for (const SpacePoint* spacePoint : m_spacePointVector) {
    spacePoint->setAssignmentState(false);
  }
  m_spacePointTagger.apply(m_filteredResults, m_spacePointVector);
}
