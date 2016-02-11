/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/topology/WireNeighborPair.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;

WireNeighborKind WireNeighborPair::getNeighborKind() const
{
  if (not getFirst() or not getSecond()) return WireNeighborKind();

  return CDCWireTopology::getInstance().getNeighborKind(*getFirst(), *getSecond());
}
