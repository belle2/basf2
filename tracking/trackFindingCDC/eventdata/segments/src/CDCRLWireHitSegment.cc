/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<const CDCWire*> CDCRLWireHitSegment::getWireSegment() const
{
  std::vector<const CDCWire*> wireSegment;
  for (const CDCRLWireHit& rlWireHit : *this) {
    wireSegment.push_back(&(rlWireHit.getWire()));
  }
  return wireSegment;
}
