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

#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/StateRejecter.icc.h>
#include <tracking/ckf/general/findlets/OnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LimitedOnStateApplier.icc.h>
#include <tracking/ckf/general/findlets/LayerToggledApplier.icc.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.icc.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

#include <framework/core/ModuleParamList.h>

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
  addProcessingSignalListener(&m_recoTrackRelator);
  addProcessingSignalListener(&m_bestMatchSelector);
  addProcessingSignalListener(&m_relationApplier);
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

  moduleParamList->getParameter<std::string>("hitsSpacePointsStoreArrayName").setDefaultValue("SVDSpacePoints");

  moduleParamList->getParameter<bool>("endEarly").setDefaultValue(false);
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

  m_recoTrackRelator.apply(m_results, m_relationsCDCToSVD);
  m_bestMatchSelector.apply(m_relationsCDCToSVD);
  m_relationApplier.apply(m_relationsCDCToSVD);
}
