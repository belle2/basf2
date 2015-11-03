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

void ConformalCDCWireHitCreator::copyHitsFromTopology(std::vector<ConformalCDCWireHit>& conformalCDCWireHitList)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const std::vector<CDCWireHit>& cdcWireHits = wireHitTopology.getWireHits();

  B2DEBUG(90, "Number of digitized hits: " << cdcWireHits.size());
  if (cdcWireHits.size() == 0) {
    B2WARNING("cdcHitsCollection is empty!");
  }

  conformalCDCWireHitList.reserve(cdcWireHits.size());
  for (const CDCWireHit& cdcWireHit : cdcWireHits) {
    if (cdcWireHit.getAutomatonCell().hasTakenFlag()) continue;
    conformalCDCWireHitList.emplace_back(cdcWireHit);
    const ConformalCDCWireHit& newlyAddedHit = conformalCDCWireHitList.back();
    if (not(newlyAddedHit.checkHitDriftLength() and newlyAddedHit.getCDCWireHit()->isAxial())) {
      conformalCDCWireHitList.pop_back();
    }
  }
  B2DEBUG(90, "Number of hits to be used by legendre track finder: " << conformalCDCWireHitList.size() << " axial.");
}


std::vector<ConformalCDCWireHit*> ConformalCDCWireHitCreator::createConformalCDCWireHitListForQT(
  std::vector<ConformalCDCWireHit>& conformalCDCWireHitList, bool useSegmentsOnly)
{
  std::vector<ConformalCDCWireHit*> QuadTreeHitWrappers;
  for (ConformalCDCWireHit& trackHit : conformalCDCWireHitList) {
    if (trackHit.getUsedFlag() or trackHit.getMaskedFlag() or (useSegmentsOnly and not trackHit.getSegment().isAxial())) {
      continue;
    }
    QuadTreeHitWrappers.push_back(&trackHit);
  }
  B2DEBUG(90, "In hit set are " << QuadTreeHitWrappers.size() << " hits.")
  return QuadTreeHitWrappers;
}
