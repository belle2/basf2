/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/segments/CDCRLWireHitSegment.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCRLWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

std::vector<const CDCWire*> CDCRLWireHitSegment::getWireSegment() const
{
  std::vector<const CDCWire*> wireSegment;
  for (const CDCRLWireHit& rlWireHit : *this) {
    wireSegment.push_back(&(rlWireHit.getWire()));
  }
  return wireSegment;
}
