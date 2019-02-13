/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/cdc/findlets/CKFToCDCFromEclFindlet.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.h>

#include <tracking/ckf/general/utilities/ClassMnemomics.h>
#include <tracking/dataobjects/RecoTrack.h>

using namespace Belle2;

CKFToCDCFromEclFindlet::~CKFToCDCFromEclFindlet() = default;

CKFToCDCFromEclFindlet::CKFToCDCFromEclFindlet()
{
  addProcessingSignalListener(&m_trackCreator);
  addProcessingSignalListener(&m_seedCreator);
  addProcessingSignalListener(&m_treeSearcher);
  addProcessingSignalListener(&m_resultFinalizer);
  addProcessingSignalListener(&m_resultStorer);
}

void CKFToCDCFromEclFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_trackCreator.exposeParameters(moduleParamList, prefix);
  m_seedCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearcher.exposeParameters(moduleParamList, prefix);
  m_resultFinalizer.exposeParameters(moduleParamList, prefix);
  m_resultStorer.exposeParameters(moduleParamList, prefix);

  // TODO: these filters might be have to set to 'all' for testing
  moduleParamList->getParameter<std::string>("statePreFilter").setDefaultValue("all");
  moduleParamList->getParameter<std::string>("stateBasicFilter").setDefaultValue("rough");
  moduleParamList->getParameter<std::string>("stateExtrapolationFilter").setDefaultValue("extrapolate_and_update");
  moduleParamList->getParameter<std::string>("stateFinalFilter").setDefaultValue("distance");
}

void CKFToCDCFromEclFindlet::beginEvent()
{
  Super::beginEvent();

  m_eclRecoTrackVector.clear();
  m_paths.clear();
  m_seeds.clear();
  m_results.clear();
}

void CKFToCDCFromEclFindlet::apply(const std::vector<TrackFindingCDC::CDCWireHit>& wireHits)
{
  m_trackCreator.apply(m_eclRecoTrackVector);
  m_seedCreator.apply(m_eclRecoTrackVector, m_seeds);

  const auto& wireHitPtrs = TrackFindingCDC::as_pointers<const TrackFindingCDC::CDCWireHit>(wireHits);

  for (const auto& seed : m_seeds) {
    B2DEBUG(100, "Starting new seed");
    m_paths.clear();
    m_paths.push_back(seed);
    m_treeSearcher.apply(m_paths, wireHitPtrs);
    m_resultFinalizer.apply(m_paths, m_results);
  }

  m_resultStorer.apply(m_results);
}
