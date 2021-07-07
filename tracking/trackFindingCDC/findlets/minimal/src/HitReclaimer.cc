/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
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
