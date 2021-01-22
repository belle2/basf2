/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Christian Wessel                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/findlets/DATCONFindlet.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/logging/Logger.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

DATCONFindlet::~DATCONFindlet() = default;

DATCONFindlet::DATCONFindlet()
{
  addProcessingSignalListener(&m_spacePointLoaderAndPreparer);

}

void DATCONFindlet::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  Super::exposeParameters(moduleParamList, prefix);

  m_spacePointLoaderAndPreparer.exposeParameters(moduleParamList, prefix);

}

void DATCONFindlet::beginEvent()
{
  Super::beginEvent();

  m_hits.clear();

}

void DATCONFindlet::apply()
{
  m_spacePointLoaderAndPreparer.apply(m_hits);
  B2DEBUG(29, "m_hits.size(): " << m_hits.size());

}
