/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/svd/findlets/CKFToSVDFindlet.h>

#include <tracking/ckf/general/findlets/SpacePointTagger.icc.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.icc.h>
#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/CKFRelationCreator.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/OverlapResolver.icc.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>
#include <tracking/ckf/general/findlets/ResultStorer.icc.h>
#include <tracking/ckf/general/utilities/Helpers.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <tracking/ckf/svd/filters/relations/LayerSVDRelationFilter.icc.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CKFToSVDFindlet::~CKFToSVDFindlet() = default;

CKFToSVDFindlet::CKFToSVDFindlet()
{
  addProcessingSignalListener(&m_dataHandler);
  addProcessingSignalListener(&m_hitsLoader);
  addProcessingSignalListener(&m_stateCreatorFromTracks);
  addProcessingSignalListener(&m_stateCreatorFromHits);
  addProcessingSignalListener(&m_relationCreator);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_spacePointTagger);
  addProcessingSignalListener(&m_resultStorer);
}

void CKFToSVDFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_dataHandler.exposeParameters(moduleParamList, prefix);
  m_hitsLoader.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromTracks.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromHits.exposeParameters(moduleParamList, prefix);
  m_relationCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_overlapResolver.exposeParameters(moduleParamList, prefix);
  m_spacePointTagger.exposeParameters(moduleParamList, prefix);
  m_resultStorer.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("minimalHitRequirement", m_param_minimalHitRequirement,
                                "Minimal Hit requirement for the results (counted in space points)",
                                m_param_minimalHitRequirement);

  moduleParamList->getParameter<std::string>("firstHighFilter").setDefaultValue("mva_with_direction_check");
  moduleParamList->getParameter<std::string>("advanceHighFilter").setDefaultValue("advance");
  moduleParamList->getParameter<std::string>("secondHighFilter").setDefaultValue("mva");
  moduleParamList->getParameter<std::string>("updateHighFilter").setDefaultValue("fit");
  moduleParamList->getParameter<std::string>("thirdHighFilter").setDefaultValue("mva");

  moduleParamList->getParameter<std::string>("filter").setDefaultValue("mva");

  moduleParamList->getParameter<std::string>("hitFilter").setDefaultValue("sensor");
  moduleParamList->getParameter<std::string>("seedFilter").setDefaultValue("all");
  moduleParamList->getParameter<std::string>("preSeedFilter").setDefaultValue("loose");
  moduleParamList->getParameter<std::string>("preHitFilter").setDefaultValue("loose");

  moduleParamList->getParameter<std::string>("hitsSpacePointsStoreArrayName").setDefaultValue("SVDSpacePoints");

  moduleParamList->getParameter<bool>("useAssignedHits").setDefaultValue(false);
}

void CKFToSVDFindlet::beginEvent()
{
  Super::beginEvent();

  // If the capacity of a std::vector is very large without being used, it just allocates RAM for no reason, increasing the RAM
  // usage unnecessarily. In this case, start with a fresh one.
  // Since std::vector.shrink() or std::vector.shrink_to_fit() not necessarily reduce the capacity in the desired way, create a
  // temporary vector of the same type, swap them to use the vector at the new location afterwards, and clear the tempoary vector.
  m_cdcRecoTrackVector.clear();
  checkResizeClear<const SpacePoint*>(m_spacePointVector, 40000);

  m_seedStates.clear();
  checkResizeClear<CKFToSVDState>(m_states, 40000);

  checkResizeClear<TrackFindingCDC::WeightedRelation<CKFToSVDState>>(m_relations, 100000);

  m_results.clear();
  m_filteredResults.clear();
}

void CKFToSVDFindlet::apply()
{
  m_dataHandler.apply(m_cdcRecoTrackVector);
  m_hitsLoader.apply(m_spacePointVector);

  B2DEBUG(29, "Now have " << m_spacePointVector.size() << " hits.");

  if (m_spacePointVector.empty() or m_cdcRecoTrackVector.empty()) {
    return;
  }

  m_stateCreatorFromTracks.apply(m_cdcRecoTrackVector, m_seedStates);
  m_stateCreatorFromHits.apply(m_spacePointVector, m_states);
  m_relationCreator.apply(m_seedStates, m_states, m_relations);

  B2DEBUG(29, "Created " << m_relations.size() << " relations.");
  m_treeSearchFindlet.apply(m_seedStates, m_states, m_relations, m_results);

  B2DEBUG(29, "Having found " << m_results.size() << " results before overlap check");

  const auto hasLowHitNumber = [this](const CKFResult<RecoTrack, SpacePoint>& result) {
    return result.getHits().size() < m_param_minimalHitRequirement;
  };
  TrackFindingCDC::erase_remove_if(m_results, hasLowHitNumber);

  m_overlapResolver.apply(m_results, m_filteredResults);

  B2DEBUG(29, "Having found " << m_filteredResults.size() << " results");

  m_resultStorer.apply(m_filteredResults);
  m_spacePointTagger.apply(m_filteredResults, m_spacePointVector);
}
