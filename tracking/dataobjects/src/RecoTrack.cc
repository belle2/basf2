/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/RecoTrack.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/WireID.h>

#include <framework/datastore/StoreArray.h>


using namespace Belle2;

ClassImp(RecoTrack);

void RecoTrack::resetHitIndices(const short pseudoCharge,
                                const Const::EDetector detector)
{
  if ((detector == Const::EDetector::CDC or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge >= 0)
    m_cdcHitIndicesPositive.clear();

  if ((detector == Const::EDetector::CDC or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge <= 0)
    m_cdcHitIndicesNegative.clear();

  if ((detector == Const::EDetector::SVD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge >= 0)
    m_svdHitIndicesPositive.clear();

  if ((detector == Const::EDetector::SVD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge <= 0)
    m_svdHitIndicesNegative.clear();

  if ((detector == Const::EDetector::PXD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge >= 0)
    m_pxdHitIndicesPositive.clear();

  if ((detector == Const::EDetector::PXD or
       detector == Const::EDetector::invalidDetector) and
      pseudoCharge <= 0)
    m_pxdHitIndicesNegative.clear();
}

void RecoTrack::fillHitPatternCDC(const short pseudoCharge)
{
  StoreArray<CDCHit> cdcHits(m_cdcHitsName);
  HitPatternCDC      hitPatternCDC(m_hitPatternCDCInitializer);
  auto& cdcHitIndices =
    pseudoCharge > 0 ? m_cdcHitIndicesPositive : m_cdcHitIndicesNegative;

  for (auto hitPair : cdcHitIndices) {
    // I need to initialize a WireID with the ID from the CDCHit to get the continuous layer ID.
    WireID wireID(cdcHits[hitPair.first]->getID());
    // Then I set the corresponding layer in the hit pattern.
    if (hitPatternCDC.setLayer(wireID.getICLayer())) {
      // Finally, if the layer was already set, I set the double layer marker.
      // This total number of layers(56) + Super Layer Number
      hitPatternCDC.setLayerFast(56 + wireID.getISuperLayer());
    }
  }
}

void RecoTrack::fillHitPatternVXD(const short pseudoCharge)
{
  //Do something similar as for the CDC
}
