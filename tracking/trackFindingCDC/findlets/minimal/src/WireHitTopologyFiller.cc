/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitTopologyFiller.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string WireHitTopologyFiller::getDescription()
{
  return
    "Publishes the wire hits also through the CDCWireHitTopology "
    "to support this legacy interface to the wire hits "
    "for modules that still rely on it.";
}

void WireHitTopologyFiller::initialize()
{
  CDCWireHitTopology::initialize();
  Super::initialize();
}

void WireHitTopologyFiller::apply(std::vector<CDCWireHit>& wireHits)
{
  auto sharedWireHits = makeUnique<ConstVectorRange<CDCWireHit>>(wireHits.begin(), wireHits.end());
  CDCWireHitTopology::getInstance().fill(std::move(sharedWireHits));
}
