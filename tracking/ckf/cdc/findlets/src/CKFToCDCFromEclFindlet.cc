/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/ckf/cdc/findlets/CKFToCDCFromEclFindlet.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <framework/core/ModuleParamList.h>

#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

using namespace Belle2;

CKFToCDCFromEclFindlet::~CKFToCDCFromEclFindlet() = default;

CKFToCDCFromEclFindlet::CKFToCDCFromEclFindlet()
{
  addProcessingSignalListener(&m_seedCreator);
  addProcessingSignalListener(&m_treeSearcher);
  addProcessingSignalListener(&m_resultFinalizer);
  addProcessingSignalListener(&m_resultStorer);
  addProcessingSignalListener(&m_duplicateRemover);
}

void CKFToCDCFromEclFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_seedCreator.exposeParameters(moduleParamList, prefix);
  m_treeSearcher.exposeParameters(moduleParamList, prefix);
  m_resultFinalizer.exposeParameters(moduleParamList, prefix);
  m_resultStorer.exposeParameters(moduleParamList, prefix);
  m_duplicateRemover.exposeParameters(moduleParamList, prefix);

  moduleParamList->getParameter<std::string>("statePreFilter").setDefaultValue("all");
  moduleParamList->getParameter<std::string>("stateBasicFilter").setDefaultValue("rough_eclSeed");
  moduleParamList->getParameter<std::string>("stateExtrapolationFilter").setDefaultValue("extrapolate_and_update");
  moduleParamList->getParameter<std::string>("stateFinalFilter").setDefaultValue("distance");

  moduleParamList->getParameter<std::string>("badTracksFilter").setDefaultValue("seedCharge");
  moduleParamList->getParameter<std::string>("duplicateTrackFilter").setDefaultValue("hitDistance");
  moduleParamList->getParameter<std::string>("duplicateSeedFilter").setDefaultValue("duplicateHits");
}

void CKFToCDCFromEclFindlet::beginEvent()
{
  Super::beginEvent();

  // Prepare the Translators (necessary to create CDCRecoHits)
  CDCRecoHit::setTranslators(new CDC::LinearGlobalADCCountTranslator(),
                             new CDC::RealisticCDCGeometryTranslator(true),
                             new CDC::RealisticTDCCountTranslator(true),
                             true);

  // Clear all vectors
  m_paths.clear();
  m_seeds.clear();
  m_results.clear();
}

void CKFToCDCFromEclFindlet::apply(const std::vector<TrackFindingCDC::CDCWireHit>& wireHits)
{
  const auto& wireHitPtrs = TrackFindingCDC::as_pointers<const TrackFindingCDC::CDCWireHit>(wireHits);

  // create the seed objects
  m_seedCreator.apply(m_seeds);

  // find the paths
  for (const auto& seed : m_seeds) {
    B2DEBUG(29, "Starting new seed");
    m_paths.clear();
    m_paths.push_back(seed);
    m_treeSearcher.apply(m_paths, wireHitPtrs);
    m_resultFinalizer.apply(m_paths, m_results);
  }

  // remove duplicate tracks (additional seeds from Bremsstrahlung)
  m_duplicateRemover.apply(m_results);

  // and store output
  m_resultStorer.apply(m_results);
}


