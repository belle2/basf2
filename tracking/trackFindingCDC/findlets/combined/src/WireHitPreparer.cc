/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
