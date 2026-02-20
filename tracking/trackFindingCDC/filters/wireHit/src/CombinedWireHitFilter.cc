/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/CombinedWireHitFilter.h>
#include <tracking/trackFindingCDC/filters/wireHit/CutsFromDBWireHitFilter.h>
#include <tracking/trackingUtilities/eventdata/hits/CDCWireHit.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackingUtilities;


CombinedWireHitFilter::CombinedWireHitFilter() : m_mvaFilter(), m_cutsFromDBFilter()
{
}

void CombinedWireHitFilter::initialize()
{
  m_cutsFromDBFilter.initialize();
  m_mvaFilter.initialize();
}

void CombinedWireHitFilter::beginRun()
{
  m_cutsFromDBFilter.beginRun();
  m_mvaFilter.beginRun();
}




Weight CombinedWireHitFilter::operator()(const CDCWireHit& wireHit)
{
  const auto* cdcHit = wireHit.getHit();
  const auto sl = cdcHit->getISuperLayer();
  if (sl <= 1) {
    return m_cutsFromDBFilter(wireHit);
  } else {
    return m_mvaFilter(wireHit);
  }
}
