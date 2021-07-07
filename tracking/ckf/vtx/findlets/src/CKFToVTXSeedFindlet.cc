/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/vtx/findlets/CKFToVTXSeedFindlet.h>

#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.icc.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CKFToVTXSeedFindlet::~CKFToVTXSeedFindlet() = default;

CKFToVTXSeedFindlet::CKFToVTXSeedFindlet()
{
  addProcessingSignalListener(&m_dataHandler);
  addProcessingSignalListener(&m_hitsLoader);
  addProcessingSignalListener(&m_stateCreatorFromTracks);
  addProcessingSignalListener(&m_stateCreatorFromHits);
  addProcessingSignalListener(&m_relationCreator);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_recoTrackRelator);
  addProcessingSignalListener(&m_bestMatchSelector);
  addProcessingSignalListener(&m_relationApplier);
}

void CKFToVTXSeedFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_dataHandler.exposeParameters(moduleParamList, prefix);
  m_hitsLoader.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromTracks.exposeParameters(moduleParamList, prefix);
  m_stateCreatorFromHits.exposeParameters(moduleParamList, prefix);
  m_relationCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearchFindlet.exposeParameters(moduleParamList, prefix);
  m_bestMatchSelector.exposeParameters(moduleParamList, prefix);
  m_recoTrackRelator.exposeParameters(moduleParamList, prefix);
  m_relationApplier.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("minimalHitRequirement", m_param_minimalHitRequirement,
                                "Minimal Hit requirement for the results (counted in space points)",
                                m_param_minimalHitRequirement);

  moduleParamList->getParameter<std::string>("firstHighFilter").setDefaultValue("non_ip_crossing");
  moduleParamList->getParameter<std::string>("advanceHighFilter").setDefaultValue("advance");
  moduleParamList->getParameter<std::string>("secondHighFilter").setDefaultValue("all");
  moduleParamList->getParameter<std::string>("updateHighFilter").setDefaultValue("fit");
  moduleParamList->getParameter<std::string>("thirdHighFilter").setDefaultValue("all");

  moduleParamList->getParameter<std::string>("hitsSpacePointsStoreArrayName").setDefaultValue("VTXSpacePoints");

  moduleParamList->getParameter<bool>("endEarly").setDefaultValue(false);
}

void CKFToVTXSeedFindlet::beginEvent()
{
  Super::beginEvent();

  m_cdcRecoTracksVector.clear();
  m_spacePointVector.clear();

  m_seedStates.clear();
  m_states.clear();
  m_relations.clear();

  m_results.clear();

  m_relationsCDCToVTX.clear();
}

void CKFToVTXSeedFindlet::apply()
{
  m_dataHandler.apply(m_cdcRecoTracksVector);
  m_hitsLoader.apply(m_spacePointVector);

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
  B2DEBUG(29, "After filtering: Having found " << m_results.size() << " results before overlap check");

  m_recoTrackRelator.apply(m_results, m_relationsCDCToVTX);
  m_bestMatchSelector.apply(m_relationsCDCToVTX);
  m_relationApplier.apply(m_relationsCDCToVTX);
}
