/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/pxd/findlets/CKFToPXDFindlet.h>

#include <tracking/ckf/general/findlets/StateCreator.icc.h>
#include <tracking/ckf/general/findlets/CKFRelationCreator.icc.h>
#include <tracking/ckf/general/findlets/StateCreatorWithReversal.icc.h>
#include <tracking/ckf/general/findlets/TreeSearcher.icc.h>
#include <tracking/ckf/general/findlets/OverlapResolver.icc.h>
#include <tracking/ckf/general/findlets/SpacePointTagger.icc.h>
#include <tracking/ckf/general/findlets/ResultStorer.icc.h>
#include <tracking/ckf/general/utilities/Helpers.h>

#include <tracking/ckf/pxd/entities/CKFToPXDResult.h>
#include <tracking/ckf/pxd/entities/CKFToPXDState.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <framework/core/ModuleParamList.h>

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
  addProcessingSignalListener(&m_resultStorer);
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
  m_resultStorer.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter("minimalHitRequirement", m_param_minimalHitRequirement,
                                "Minimal Hit requirement for the results (counted in space points)",
                                m_param_minimalHitRequirement);
  moduleParamList->addParameter("onlyUseTracksWithSVD", m_param_onlyUseTracksWithSVD,
                                "Only use tracks which have an SVD hit associated.",
                                m_param_onlyUseTracksWithSVD);
  moduleParamList->addParameter("reverseSeedState", m_param_reverseSeed, "Reverse the seed.", m_param_reverseSeed);

  // Default values
  moduleParamList->getParameter<std::string>("advanceHighFilter").setDefaultValue("advance");
  moduleParamList->getParameter<std::string>("updateHighFilter").setDefaultValue("fit");

  moduleParamList->getParameter<std::string>("firstHighFilter").setDefaultValue("mva_with_direction_check");
  moduleParamList->getParameter<std::string>("secondHighFilter").setDefaultValue("mva");
  moduleParamList->getParameter<std::string>("thirdHighFilter").setDefaultValue("mva");

  moduleParamList->getParameter<bool>("useAssignedHits").setDefaultValue(false);

  moduleParamList->getParameter<std::string>("hitFilter").setDefaultValue("sensor");
  moduleParamList->getParameter<std::string>("seedFilter").setDefaultValue("sensor");
  moduleParamList->getParameter<std::string>("preSeedFilter").setDefaultValue("loose");
  moduleParamList->getParameter<std::string>("preHitFilter").setDefaultValue("loose");

  moduleParamList->getParameter<std::string>("hitsSpacePointsStoreArrayName").setDefaultValue("PXDSpacePoints");

  moduleParamList->getParameter<std::string>("filter").setDefaultValue("mva");
}

void CKFToPXDFindlet::beginEvent()
{
  Super::beginEvent();

  // If the capacity of a std::vector is very large without being used, it just allocates RAM for no reason,
  // increasing the RAM usage unnecessarily. In this case, start with a fresh one.
  // Since std::vector.shrink() or std::vector.shrink_to_fit() not necessarily reduce the capacity in the desired way,
  // create a temporary vector of the same type and swap them to use the vector at the new location afterwards.
  m_recoTracksVector.clear();
  checkAndResize<const SpacePoint*>(m_spacePointVector, 2000);

  m_seedStates.clear();
  checkAndResize<CKFToPXDState>(m_states, 2000);

  checkAndResize<TrackFindingCDC::WeightedRelation<CKFToPXDState>>(m_relations, 2000);

  m_results.clear();
  m_filteredResults.clear();
}

void CKFToPXDFindlet::apply()
{
  m_dataHandler.apply(m_recoTracksVector);
  m_hitsLoader.apply(m_spacePointVector);

  if (m_spacePointVector.empty() or m_recoTracksVector.empty()) {
    return;
  }

  // Delete stuff not from the PXD
  const auto notFromPXD = [](const SpacePoint * spacePoint) {
    return spacePoint->getType() != VXD::SensorInfoBase::PXD;
  };
  TrackFindingCDC::erase_remove_if(m_spacePointVector, notFromPXD);

  if (m_param_onlyUseTracksWithSVD) {
    const auto hasNoSVD = [this](const RecoTrack * recoTrack) {
      const auto& svdHitList = recoTrack->getSortedSVDHitList();
      if (svdHitList.empty()) return true;
      // Require at least one hit in layer 3 or 4
      return m_param_reverseSeed ? svdHitList.back()->getSensorID().getLayerNumber() > 4
             : svdHitList.front()->getSensorID().getLayerNumber() > 4;
    };
    TrackFindingCDC::erase_remove_if(m_recoTracksVector, hasNoSVD);
  }

  B2DEBUG(29, "Now have " << m_spacePointVector.size() << " hits.");

  m_stateCreatorFromTracks.apply(m_recoTracksVector, m_seedStates);
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
