/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/findlets/minimal/HitReclaimer.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

HitReclaimer::HitReclaimer() = default;

void HitReclaimer::initialize()
{
  Super::initialize();
}

std::string HitReclaimer::getDescription()
{
  return "A small findlet that removes flags from hits not accepted by conventional tracking.";
}

void HitReclaimer::apply(const std::vector<CDCWireHit>& wireHits)
{
  for (const CDCWireHit& wireHit : wireHits) {
    if (wireHit->hasBackgroundFlag() or wireHit->hasMaskedFlag()) {
      wireHit->unsetTakenFlag();
    }
  }
}
