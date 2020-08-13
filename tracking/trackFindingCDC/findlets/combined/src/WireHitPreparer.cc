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
  this->addProcessingSignalListener(&m_wireHitBackgroundDetector);
  this->addProcessingSignalListener(&m_wireHitMCMultiLoopBlocker);
  this->addProcessingSignalListener(&m_asicBackgroundDetector);
}

std::string WireHitPreparer::getDescription()
{
  return "Combine the CDCHits from the DataStore with the geometry information.";
}

void WireHitPreparer::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  m_wireHitCreator.exposeParameters(moduleParamList, prefix);
  m_wireHitBackgroundBlocker.exposeParameters(moduleParamList, prefix);
  m_wireHitBackgroundDetector.exposeParameters(moduleParamList, prefix);
  m_wireHitMCMultiLoopBlocker.exposeParameters(moduleParamList, prefix);
  m_asicBackgroundDetector.exposeParameters(moduleParamList, prefix);
}

void WireHitPreparer::apply(std::vector<CDCWireHit>& outputWireHits)
{
  m_wireHitCreator.apply(outputWireHits);
  m_wireHitBackgroundBlocker.apply(outputWireHits);
  m_wireHitBackgroundDetector.apply(outputWireHits);
  m_wireHitMCMultiLoopBlocker.apply(outputWireHits);
  m_asicBackgroundDetector.apply(outputWireHits);
}
