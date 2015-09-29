/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRLWireHitPair::CDCRLWireHitPair(const CDCRLWireHitPair& rlWireHitPair):
  CDCRLWireHitPair(rlWireHitPair.m_fromRLWireHit, rlWireHitPair.m_toRLWireHit)
{
}

CDCRLWireHitPair::CDCRLWireHitPair(const CDCRLWireHit* fromRLWireHit,
                                   const CDCRLWireHit* toRLWireHit):
  m_fromRLWireHit(fromRLWireHit),
  m_toRLWireHit(toRLWireHit)
{
  assert(fromRLWireHit);
  assert(toRLWireHit);
}

CDCRLWireHitPair CDCRLWireHitPair::reversed() const
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const CDCRLWireHit* newFromRLWireHit = wireHitTopology.getReverseOf(getToRLWireHit());
  const CDCRLWireHit* newToRLWireHit = wireHitTopology.getReverseOf(getFromRLWireHit());

  return CDCRLWireHitPair(newFromRLWireHit, newToRLWireHit);
}

void CDCRLWireHitPair::reverse()
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const CDCRLWireHit* newFromRLWireHit = wireHitTopology.getReverseOf(getToRLWireHit());
  const CDCRLWireHit* newToRLWireHit = wireHitTopology.getReverseOf(getFromRLWireHit());

  setFromRLWireHit(newFromRLWireHit);
  setToRLWireHit(newToRLWireHit);
}

void CDCRLWireHitPair::setFromRLInfo(const ERightLeft fromRLInfo)
{
  if (fromRLInfo != getFromRLInfo()) {
    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
    const CDCRLWireHit* newFromRLWireHit = wireHitTopology.getReverseOf(getFromRLWireHit());
    setFromRLWireHit(newFromRLWireHit);
  }
}

void CDCRLWireHitPair::setToRLInfo(const ERightLeft toRLInfo)
{
  if (toRLInfo != getToRLInfo()) {
    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
    const CDCRLWireHit* newToRLWireHit = wireHitTopology.getReverseOf(getToRLWireHit());
    setToRLWireHit(newToRLWireHit);
  }
}
