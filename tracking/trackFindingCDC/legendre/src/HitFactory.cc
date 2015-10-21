/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/HitFactory.h>
#include <tracking/trackFindingCDC/legendre/HitProcessor.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <cdc/dataobjects/CDCHit.h>


#include <framework/datastore/StoreArray.h>

#include <genfit/TrackCand.h>
#include <framework/gearbox/Const.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


void HitFactory::initializeQuadTreeHitWrappers()
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  const std::vector<CDCWireHit>& cdcWireHits = wireHitTopology.getWireHits();

  B2DEBUG(90, "Number of digitized hits: " << cdcWireHits.size());
  if (cdcWireHits.size() == 0) {
    B2WARNING("cdcHitsCollection is empty!");
  }

  m_QuadTreeHitWrappers.reserve(cdcWireHits.size());
  for (const CDCWireHit& cdcWireHit : cdcWireHits) {
    if (cdcWireHit.getAutomatonCell().hasTakenFlag()) continue;
    QuadTreeHitWrapper QuadTreeHitWrapper(cdcWireHit);
    if (QuadTreeHitWrapper.checkHitDriftLength() and QuadTreeHitWrapper.getCDCWireHit()->isAxial()) {
      m_QuadTreeHitWrappers.push_back(std::move(QuadTreeHitWrapper));
    }
  }
  B2DEBUG(90, "Number of hits to be used by legendre track finder: " << m_QuadTreeHitWrappers.size() << " axial.");

}


std::vector<QuadTreeHitWrapper*> HitFactory::createQuadTreeHitWrappersForQT(bool useSegmentsOnly)
{
  std::vector<QuadTreeHitWrapper*> QuadTreeHitWrappers;
  doForAllHits([&QuadTreeHitWrappers, &useSegmentsOnly](QuadTreeHitWrapper & trackHit) {
    if (trackHit.getUsedFlag() || trackHit.getMaskedFlag()) return;
    if ((not trackHit.getSegment().isAxial()) and useSegmentsOnly) return;
    QuadTreeHitWrappers.push_back(&trackHit);
  });
  B2DEBUG(90, "In hit set are " << QuadTreeHitWrappers.size() << " hits.")
  return QuadTreeHitWrappers;
}

void HitFactory::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
  }
}

std::vector<const CDCWireHit*> HitFactory::convertQTHitsToWireHits(std::vector<QuadTreeHitWrapper>& qtHitsToConvert)
{
  std::vector<const CDCWireHit*> cdcWireHits;

  for (QuadTreeHitWrapper& hit : qtHitsToConvert) {
    cdcWireHits.push_back(hit.getCDCWireHit());
  }

  return cdcWireHits;
}

std::vector<QuadTreeHitWrapper> HitFactory::convertWireHitsToQTHits(std::vector<const CDCWireHit*>& wireHitsToConvert)
{
  std::vector<QuadTreeHitWrapper> qtHits;

  for (QuadTreeHitWrapper& qtHit : m_QuadTreeHitWrappers) {
    for (const CDCWireHit* wireHit : wireHitsToConvert) {
      if (qtHit.getCDCWireHit()->getStoreIHit() == wireHit->getStoreIHit()) qtHits.push_back(qtHit);
    }
  }

  return qtHits;
}
