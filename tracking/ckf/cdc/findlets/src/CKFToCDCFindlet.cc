/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <tracking/ckf/cdc/findlets/CKFToCDCFindlet.h>

#include <tracking/trackingUtilities/utilities/Algorithms.h>

#include <framework/database/DBObjPtr.h>
#include <framework/core/ModuleParamList.h>

#include <tracking/dbobjects/SVDToCDCCKFParameters.h>

using namespace Belle2;

CKFToCDCFindlet::~CKFToCDCFindlet() = default;

CKFToCDCFindlet::CKFToCDCFindlet()
{
  addProcessingSignalListener(&m_trackHandler);
  addProcessingSignalListener(&m_seedCreator);
  addProcessingSignalListener(&m_treeSearcher);
  addProcessingSignalListener(&m_resultFinalizer);
  addProcessingSignalListener(&m_resultStorer);
}

void CKFToCDCFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_trackHandler.exposeParameters(moduleParamList, prefix);
  m_seedCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearcher.exposeParameters(moduleParamList, prefix);
  m_resultFinalizer.exposeParameters(moduleParamList, prefix);
  m_resultStorer.exposeParameters(moduleParamList, prefix);

  moduleParamList->getParameter<std::string>("statePreFilter").setDefaultValue("all");
  moduleParamList->getParameter<std::string>("stateBasicFilter").setDefaultValue("rough");
  moduleParamList->getParameter<std::string>("stateExtrapolationFilter").setDefaultValue("extrapolate_and_update");
  moduleParamList->getParameter<std::string>("stateFinalFilter").setDefaultValue("distance");
}

void CKFToCDCFindlet::beginRun()
{
  Super::beginRun();

  DBObjPtr<SVDToCDCCKFParameters> payload;

  if (!payload.isValid()) {
    B2FATAL("CKFToCDCFindlet: DB payload 'SVDToCDCCKFParameters' not found or not valid for current run.");
  }

  // Payload parameters (INTs, FLOATs)
  m_trackHandler.setMinimalPtRequirement(payload->getMinimalPtRequirement());

  m_treeSearcher.setMaximalDeltaPhi(payload->getMaximalDeltaPhi());
  m_treeSearcher.setMaximalLayerJump(payload->getMaximalLayerJump());
  m_treeSearcher.setMaximalLayerJumpBackwardSeed(payload->getMaximalLayerJumpBackwardSeed());
  m_treeSearcher.setPathMaximalCandidatesInFlight(payload->getPathMaximalCandidatesInFlight());
  m_treeSearcher.setStateMaximalHitCandidates(payload->getStateMaximalHitCandidates());

  B2DEBUG(20, "CKFToCDCFindlet: Loaded and applied parameters from DB payload 'SVDToCDCCKFParameters'.");
}

void CKFToCDCFindlet::beginEvent()
{
  Super::beginEvent();

  m_vxdRecoTrackVector.clear();
  m_paths.clear();
  m_seeds.clear();
  m_results.clear();
}

void CKFToCDCFindlet::apply(const std::vector<TrackingUtilities::CDCWireHit>& wireHits)
{
  m_trackHandler.apply(m_vxdRecoTrackVector);
  m_seedCreator.apply(m_vxdRecoTrackVector, m_seeds);

  const auto& wireHitPtrs = TrackingUtilities::as_pointers<const TrackingUtilities::CDCWireHit>(wireHits);

  for (const auto& seed : m_seeds) {
    B2DEBUG(29, "Starting new seed");
    m_paths.clear();
    m_paths.push_back(seed);
    m_treeSearcher.apply(m_paths, wireHitPtrs);
    m_resultFinalizer.apply(m_paths, m_results);
  }

  m_resultStorer.apply(m_results);
}
