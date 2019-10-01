/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Cyrille Praz                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundDetector.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

WireHitBackgroundDetector::WireHitBackgroundDetector()
{
  /// adding the filter as a subordinary processing signal listener.
  this->addProcessingSignalListener(&m_wireHitFilter);
}

std::string WireHitBackgroundDetector::getDescription()
{
  return "Marks hits as background based on the result of a filter.";
}

void WireHitBackgroundDetector::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  m_wireHitFilter.exposeParameters(moduleParamList, prefix);
}

void WireHitBackgroundDetector::apply(std::vector<CDCWireHit>& wireHits)
{
  for (CDCWireHit& wireHit : wireHits) {

    Weight wireHitWeight = m_wireHitFilter(wireHit);

    if (std::isnan(wireHitWeight)) {
      wireHit->setBackgroundFlag();
      wireHit->setTakenFlag();
      wireHit->setBadADCOrTOTFlag();
    }
  }
}
