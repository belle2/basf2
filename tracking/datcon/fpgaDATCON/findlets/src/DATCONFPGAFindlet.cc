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

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONFPGAFindlet::~DATCONFPGAFindlet() = default;

DATCONFPGAFindlet::DATCONFPGAFindlet()
{
  addProcessingSignalListener(&m_clusterLoader);
}

void DATCONFPGAFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_clusterLoader.exposeParameters(moduleParamList, prefix);
}

void DATCONFPGAFindlet::beginEvent()
{
  Super::beginEvent();

  m_uHits.clear();
  m_vHits.clear();
}

void DATCONFPGAFindlet::apply()
{
//   B2INFO("Here I am, this is me");
  m_clusterLoader.apply(m_uHits, m_vHits);

  // hit vectors are empty in case of high occupancy, a warning is created in m_clusterLoader
  if (m_uHits.empty() or m_vHits.empty()) {
    return;
  }

}
