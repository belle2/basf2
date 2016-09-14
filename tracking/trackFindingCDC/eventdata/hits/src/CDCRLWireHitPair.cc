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

CDCRLWireHitPair::CDCRLWireHitPair(const CDCRLWireHit& fromRLWireHit,
                                   const CDCRLWireHit& toRLWireHit,
                                   int iCluster)
  : m_fromRLWireHit(fromRLWireHit)
  , m_toRLWireHit(toRLWireHit)
  , m_iCluster(iCluster)
{
}

CDCRLWireHitPair CDCRLWireHitPair::reversed() const
{
  return CDCRLWireHitPair(getToRLWireHit().reversed(), getFromRLWireHit().reversed(), getICluster());
}

void CDCRLWireHitPair::reverse()
{
  std::swap(m_fromRLWireHit, m_toRLWireHit);
  m_fromRLWireHit.reverse();
  m_toRLWireHit.reverse();
}

CDCRLWireHitPair CDCRLWireHitPair::getAlias() const
{
  return CDCRLWireHitPair(getFromRLWireHit().getAlias(), getToRLWireHit().getAlias(), getICluster());
}

void CDCRLWireHitPair::setFromRLInfo(ERightLeft fromRLInfo)
{
  m_fromRLWireHit.setRLInfo(fromRLInfo);
}

void CDCRLWireHitPair::setToRLInfo(ERightLeft toRLInfo)
{
  m_toRLWireHit.setRLInfo(toRLInfo);
}
