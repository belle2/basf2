/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitBackgroundBlocker.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string WireHitBackgroundBlocker::getDescription()
{
  return "Marks hits as background based on simple heuristics.";
}

void WireHitBackgroundBlocker::exposeParameters(ModuleParamList* moduleParamList,
                                                const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "blockNegativeDriftLength"),
                                m_param_blockNegativeDriftLength,
                                "Switch to drop wire hits with negative "
                                "drift lengths from output",
                                m_param_blockNegativeDriftLength);

  moduleParamList->addParameter(prefixed(prefix, "noiseChargeDeposit"),
                                m_param_noiseChargeDeposit,
                                "Threshold energy below which the hit is considered "
                                "to be electronic noise",
                                m_param_noiseChargeDeposit);
}

void WireHitBackgroundBlocker::apply(std::vector<CDCWireHit>& wireHits)
{
  for (CDCWireHit& wireHit : wireHits) {
    bool markAsBackground = false;

    if (m_param_blockNegativeDriftLength) {
      if (wireHit.getRefDriftLength() < 0) {
        markAsBackground = true;
      }
    }

    if (wireHit.getRefChargeDeposit() < m_param_noiseChargeDeposit) {
      markAsBackground = true;
    }

    if (markAsBackground) {
      wireHit->setBackgroundFlag();
      wireHit->setTakenFlag();
    }
  }
}
