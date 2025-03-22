/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitSegment.h>

#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;

std::vector<const CDCWire*> CDCWireHitSegment::getWireSegment() const
{
  std::vector<const CDCWire*> wireSegment;
  for (const CDCWireHit* ptrWireHit : *this) {
    wireSegment.push_back(&(ptrWireHit->getWire()));
  }
  return wireSegment;
}
