/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;

CDCRLWireHit::CDCRLWireHit(const CDCWireHit* wireHit,
                           ERightLeft rlInfo) :
  CDCRLTaggedWireHit(wireHit, rlInfo)
{
}

const CDCRLWireHit* CDCRLWireHit::reversed() const
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  return wireHitTopology.getReverseOf(*this);
}
