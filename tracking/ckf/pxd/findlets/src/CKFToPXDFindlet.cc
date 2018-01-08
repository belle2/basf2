/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/pxd/findlets/CKFToPXDFindlet.h>

#include <tracking/ckf/general/findlets/CKFDataHandler.icc.h>
#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/CKFRelationCreator.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/OverlapResolver.icc.h>
#include <tracking/ckf/general/findlets/SpacePointTagger.icc.h>

#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <framework/core/ModuleParamList.dcl.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CKFToPXDFindlet::~CKFToPXDFindlet() = default;

CKFToPXDFindlet::CKFToPXDFindlet()
{
  addProcessingSignalListener(&m_dataHandler);
  addProcessingSignalListener(&m_hitsLoader);
  addProcessingSignalListener(&m_stateCreatorFromTracks);
  addProcessingSignalListener(&m_stateCreatorFromHits);
  addProcessingSignalListener(&m_relationCreator);
  addProcessingSignalListener(&m_treeSearchFindlet);
  addProcessingSignalListener(&m_overlapResolver);
  addProcessingSignalListener(&m_spacePointTagger);
}

void CKFToPXDFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
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

  moduleParamList->addParameter("minimalHitRequirement", m_param_minimalHitRequirement,
                                "Minimal Hit requirement for the results (counted in space points)",
                                m_param_minimalHitRequirement);

  // Default values
  moduleParamList->getParameter<std::string>("advanceHighFilter").setDefaultValue("advance");
  moduleParamList->getParameter<std::string>("updateHighFilter").setDefaultValue("fit");

  moduleParamList->getParameter<std::string>("firstHighFilter").setDefaultValue("mva");
  moduleParamList->getParameter<std::string>("secondHighFilter").setDefaultValue("mva");
  moduleParamList->getParameter<std::string>("thirdHighFilter").setDefaultValue("mva");

  moduleParamList->getParameter<bool>("useAssignedHits").setDefaultValue(false);

  moduleParamList->getParameter<std::string>("hitFilter").setDefaultValue("sensor");
  moduleParamList->getParameter<std::string>("seedFilter").setDefaultValue("sensor");

  moduleParamList->getParameter<std::string>("filter").setDefaultValue("mva");
}

void CKFToPXDFindlet::beginEvent()
{
  Super::beginEvent();

  m_recoTracksVector.clear();
  m_spacePointVector.clear();

  m_seedStates.clear();
  m_states.clear();
  m_relations.clear();

  m_results.clear();
  m_filteredResults.clear();
}

void CKFToPXDFindlet::apply()
{
  m_dataHandler.apply(m_recoTracksVector);
  m_hitsLoader.apply(m_spacePointVector);

  // Delete stuff not from the PXD
  const auto notFromPXD = [](const SpacePoint * spacePoint) {
    return spacePoint->getType() != VXD::SensorInfoBase::PXD;
  };
  TrackFindingCDC::erase_remove_if(m_spacePointVector, notFromPXD);

  const auto hasNoSVD = [](const RecoTrack * recoTrack) {
    const auto& svdHitList = recoTrack->getSortedSVDHitList();
    return svdHitList.empty() or svdHitList.front()->getSensorID().getLayerNumber() > 4;
  };
  TrackFindingCDC::erase_remove_if(m_recoTracksVector, hasNoSVD);

  B2DEBUG(50, "Now have " << m_spacePointVector.size() << " hits.");

  m_stateCreatorFromTracks.apply(m_recoTracksVector, m_seedStates);
  m_stateCreatorFromHits.apply(m_spacePointVector, m_states);
  m_relationCreator.apply(m_seedStates, m_states, m_relations);

  B2DEBUG(50, "Created " << m_relations.size() << " relations.");

  m_treeSearchFindlet.apply(m_seedStates, m_states, m_relations, m_results);

  B2DEBUG(50, "Having found " << m_results.size() << " results before overlap check");

  const auto hasLowHitNumber = [this](const CKFResult<RecoTrack, SpacePoint>& result) {
    return result.getHits().size() < m_param_minimalHitRequirement;
  };
  TrackFindingCDC::erase_remove_if(m_results, hasLowHitNumber);

  m_overlapResolver.apply(m_results, m_filteredResults);

  B2DEBUG(50, "Having found " << m_filteredResults.size() << " results");

  m_dataHandler.store(m_filteredResults);
  m_spacePointTagger.apply(m_filteredResults, m_spacePointVector);
}
