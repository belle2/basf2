/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/vtx/findlets/CKFToVTXFindlet.h>

#include <tracking/ckf/general/findlets/SpacePointTagger.icc.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.icc.h>
#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/CKFRelationCreator.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/OverlapResolver.icc.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>
#include <tracking/ckf/general/findlets/ResultStorer.icc.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>
#include <tracking/ckf/vtx/filters/relations/LayerVTXRelationFilter.icc.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CKFToVTXFindlet::~CKFToVTXFindlet() = default;

CKFToVTXFindlet::CKFToVTXFindlet()
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

void CKFToVTXFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
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

  // Default values
  moduleParamList->getParameter<std::string>("firstHighFilter").setDefaultValue("mva_with_direction_check");
  moduleParamList->getParameter<std::string>("advanceHighFilter").setDefaultValue("advance");
  moduleParamList->getParameter<std::string>("secondHighFilter").setDefaultValue("mva");
  moduleParamList->getParameter<std::string>("updateHighFilter").setDefaultValue("fit");
  moduleParamList->getParameter<std::string>("thirdHighFilter").setDefaultValue("mva");

  moduleParamList->getParameter<bool>("useAssignedHits").setDefaultValue(false);

  moduleParamList->getParameter<std::string>("hitFilter").setDefaultValue("sensor");
  moduleParamList->getParameter<std::string>("seedFilter").setDefaultValue("all");
  moduleParamList->getParameter<std::string>("preSeedFilter").setDefaultValue("loose");
  moduleParamList->getParameter<std::string>("preHitFilter").setDefaultValue("loose");

  moduleParamList->getParameter<std::string>("hitsSpacePointsStoreArrayName").setDefaultValue("VTXSpacePoints");

  moduleParamList->getParameter<std::string>("filter").setDefaultValue("mva");
}

void CKFToVTXFindlet::beginEvent()
{
  Super::beginEvent();

  m_cdcRecoTracksVector.clear();
  m_spacePointVector.clear();

  m_seedStates.clear();
  m_states.clear();
  m_relations.clear();

  m_results.clear();
  m_filteredResults.clear();
}

void CKFToVTXFindlet::apply()
{
  m_dataHandler.apply(m_cdcRecoTracksVector);
  m_hitsLoader.apply(m_spacePointVector);

  B2DEBUG(29, "Now have " << m_spacePointVector.size() << " hits.");

  if (m_spacePointVector.empty() or m_cdcRecoTracksVector.empty()) {
    return;
  }

  m_stateCreatorFromTracks.apply(m_cdcRecoTracksVector, m_seedStates);
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
