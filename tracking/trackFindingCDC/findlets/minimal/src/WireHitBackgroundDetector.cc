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

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string WireHitBackgroundDetector::getDescription()
{
  return "Marks hits as background based on the result of a filter.";
}

void WireHitBackgroundDetector::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  //m_wireHitFilter.exposeParameters(moduleParamList, prefix);

  moduleParamList->addParameter(prefixed(prefix, "Threshold"),
                                m_wireHitFilter,
                                "Threshold energy below which the hit is considered "
                                "to be electronic noise",
                                m_wireHitFilter);
}

void WireHitBackgroundDetector::apply(std::vector<CDCWireHit>& wireHits)
{
  for (CDCWireHit& wireHit : wireHits) {
    bool markAsBackground = false;

    if (wireHit.getRefChargeDeposit() < m_wireHitFilter) {
      markAsBackground = true;
    }

    if (markAsBackground) {
      wireHit->setBackgroundFlag();
      wireHit->setTakenFlag();
    }
  }
}
