/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/combined/WireHitPreparer.h>

using namespace Belle2;
using namespace TrackFindingCDC;

WireHitPreparer::WireHitPreparer()
{
  this->addProcessingSignalListener(&m_wireHitCreator);
  this->addProcessingSignalListener(&m_wireHitBackgroundBlocker);
  this->addProcessingSignalListener(&m_wireHitMCMultiLoopBlocker);
}

std::string WireHitPreparer::getDescription()
{
  return "Combine the CDCHits from the DataStore with the geometry information.";
}

void WireHitPreparer::exposeParams(ParamList* paramList, const std::string& prefix)
{
  m_wireHitCreator.exposeParams(paramList, prefix);
  m_wireHitBackgroundBlocker.exposeParams(paramList, prefix);
  m_wireHitMCMultiLoopBlocker.exposeParams(paramList, prefix);
}

void WireHitPreparer::apply(std::vector<CDCWireHit>& outputWireHits)
{
  m_wireHitCreator.apply(outputWireHits);
  m_wireHitBackgroundBlocker.apply(outputWireHits);
  m_wireHitMCMultiLoopBlocker.apply(outputWireHits);
}
