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

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCRLWireHitPair::CDCRLWireHitPair(const CDCRLTaggedWireHit& fromRLWireHit,
                                   const CDCRLTaggedWireHit& toRLWireHit):
  m_fromRLWireHit(fromRLWireHit),
  m_toRLWireHit(toRLWireHit)
{
}

CDCRLWireHitPair CDCRLWireHitPair::reversed() const
{
  return CDCRLWireHitPair(m_toRLWireHit.reversed(), m_fromRLWireHit.reversed());
}

void CDCRLWireHitPair::reverse()
{
  std::swap(m_fromRLWireHit, m_toRLWireHit);
  m_fromRLWireHit.reverse();
  m_toRLWireHit.reverse();
}

void CDCRLWireHitPair::setFromRLInfo(ERightLeft fromRLInfo)
{
  m_fromRLWireHit.setRLInfo(fromRLInfo);
}

void CDCRLWireHitPair::setToRLInfo(ERightLeft toRLInfo)
{
  m_toRLWireHit.setRLInfo(toRLInfo);
}
