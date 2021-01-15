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
  addProcessingSignalListener(&m_clusterLoader);
  addProcessingSignalListener(&m_uInterceptFinder);
  addProcessingSignalListener(&m_vInterceptFinder);
}

void DATCONFPGAFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_clusterLoader.exposeParameters(moduleParamList, prefix);
  m_uInterceptFinder.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "uSide"));
  m_vInterceptFinder.exposeParameters(moduleParamList, TrackFindingCDC::prefixed(prefix, "vSide"));
}

void DATCONFPGAFindlet::beginEvent()
{
  Super::beginEvent();

  m_uHits.clear();
  m_vHits.clear();
  m_uTracks.clear();
  m_vTracks.clear();

}

void DATCONFPGAFindlet::apply()
{
  m_clusterLoader.apply(m_uHits, m_vHits);
//   B2INFO("m_uHits.size(): " << m_uHits.size() << " m_vHits.size(): " << m_vHits.size());

  // hit vectors are empty in case of high occupancy, a warning is created in m_clusterLoader
  if (m_uHits.empty() or m_vHits.empty()) {
    return;
  }

  m_uInterceptFinder.apply(m_uHits, m_uTracks);
  m_vInterceptFinder.apply(m_vHits, m_vTracks);

//   B2INFO("m_uTracks.size(): " << m_uTracks.size() << " m_vTracks.size(): " << m_vTracks.size());

}
