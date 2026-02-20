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


CombinedWireHitFilter::CombinedWireHitFilter()
{
}

void CombinedWireHitFilter::initialize()
{
}

void CombinedWireHitFilter::beginRun()
{
}



Weight CombinedWireHitFilter::operator()(const CDCWireHit& wireHit)
{
}
