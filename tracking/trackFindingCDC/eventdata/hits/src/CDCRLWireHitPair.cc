/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHitPair.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/topology/WireNeighborKind.h>
#include <tracking/trackFindingCDC/topology/CDCWire.h>

#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

#include <utility>

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

// Inline candidate
inline WireNeighborKind CDCRLWireHitPair::getNeighborKind() const
{
  return getFromWire().getNeighborKind(getToWire());
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
