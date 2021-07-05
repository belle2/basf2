/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/wireHit/AllWireHitFilter.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

Weight AllWireHitFilter::operator()(const CDCWireHit& wireHit)
{
  return (*wireHit.getHit()).getADCCount();
}
