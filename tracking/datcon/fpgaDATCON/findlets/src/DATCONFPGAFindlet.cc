/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/fpgaDATCON/findlets/DATCONFPGAFindlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONFPGAFindlet::~DATCONFPGAFindlet() = default;

DATCONFPGAFindlet::DATCONFPGAFindlet()
{
  addProcessingSignalListener(&m_digitConverter);
  addProcessingSignalListener(&m_uClusterizer);
  addProcessingSignalListener(&m_vClusterizer);

  addProcessingSignalListener(&m_clusterLoaderAndPreparer);
  addProcessingSignalListener(&m_clusterLoaderAndPreparer2);

  addProcessingSignalListener(&m_uInterceptFinder);
  addProcessingSignalListener(&m_vInterceptFinder);
}

void DATCONFPGAFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_digitConverter.exposeParameters(moduleParamList, prefix);
  m_uClusterizer.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "uSide"));
  m_vClusterizer.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "vSide"));

  m_clusterLoaderAndPreparer.exposeParameters(moduleParamList, prefix);
  m_clusterLoaderAndPreparer2.exposeParameters(moduleParamList, prefix);

  m_uInterceptFinder.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "uSide"));
  m_vInterceptFinder.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "vSide"));
}

void DATCONFPGAFindlet::beginEvent()
{
  Super::beginEvent();

  m_uDigits.clear();
  m_vDigits.clear();
  m_uClusters.clear();
  m_vClusters.clear();
  m_uHits.clear();
  m_vHits.clear();
  m_uTracks.clear();
  m_vTracks.clear();

}

void DATCONFPGAFindlet::apply()
{
  m_digitConverter.apply(m_uDigits, m_vDigits);
  B2DEBUG(29, "m_uDigits.size(): " << m_uDigits.size() << " m_vDigits.size(): " << m_vDigits.size());

  m_uClusterizer.apply(m_uDigits, m_uClusters);
  m_vClusterizer.apply(m_vDigits, m_vClusters);
  B2DEBUG(29, "m_uClusters.size(): " << m_uClusters.size() << " m_vClusters.size(): " << m_vClusters.size());

//   m_clusterLoaderAndPreparer.apply(m_uHits, m_vHits);
//   B2DEBUG(29, "1: m_uHits.size(): " << m_uHits.size() << " m_vHits.size(): " << m_vHits.size());
  m_clusterLoaderAndPreparer2.apply(m_uClusters, m_vClusters, m_uHits, m_vHits);
  B2DEBUG(29, "2: m_uHits.size(): " << m_uHits.size() << " m_vHits.size(): " << m_vHits.size());

  // hit vectors are empty in case of high occupancy, a warning is created in m_clusterLoader
  if (m_uHits.empty() or m_vHits.empty()) {
    return;
  }

  m_uInterceptFinder.apply(m_uHits, m_uTracks);
  m_vInterceptFinder.apply(m_vHits, m_vTracks);

  B2DEBUG(29, "m_uTracks.size(): " << m_uTracks.size() << " m_vTracks.size(): " << m_vTracks.size());

}
