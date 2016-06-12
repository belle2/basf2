/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/creators/ConformalCDCWireHitCreator.h>

#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<ConformalCDCWireHit*> ConformalCDCWireHitCreator::createConformalCDCWireHitListForQT(
  std::vector<ConformalCDCWireHit>& conformalCDCWireHitList)
{
  std::vector<ConformalCDCWireHit*> QuadTreeHitWrappers;
  for (ConformalCDCWireHit& trackHit : conformalCDCWireHitList) {
    if (trackHit.getUsedFlag() or trackHit.getMaskedFlag()) {
      continue;
    }
    QuadTreeHitWrappers.push_back(&trackHit);
  }
  B2DEBUG(90, "In hit set are " << QuadTreeHitWrappers.size() << " hits.")
  return QuadTreeHitWrappers;
}
