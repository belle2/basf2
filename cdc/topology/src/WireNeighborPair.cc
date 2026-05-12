/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <cdc/topology/WireNeighborPair.h>

#include <cdc/topology/CDCWireTopology.h>

using namespace Belle2;
using namespace CDC;

WireNeighborKind WireNeighborPair::getNeighborKind() const
{
  if (not getFirst() or not getSecond()) return WireNeighborKind();

  return CDCWireTopology::getInstance().getNeighborKind(*getFirst(), *getSecond());
}
