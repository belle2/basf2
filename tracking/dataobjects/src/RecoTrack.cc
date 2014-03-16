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
#include <framework/logging/Logger.h>

#include <algorithm>
#include <utility>


using namespace Belle2;
using namespace std;

ClassImp(RecoTrack);

RecoTrack::RecoTrack(const std::string& cdcHitsName,
                     const std::string& svdHitsName,
                     const std::string& pxdHitsName,
                     const SorterBaseCDCHit sorterBaseCDCHit,
                     const SorterBaseVXDHit sorterBaseVXDHit) :
  m_hitPatternCDCInitializer(0), m_hitPatternVXDInitializer(0),
  m_cdcHitsName(cdcHitsName), m_assumeSortedCDC(false),
  m_sorterBaseCDC(sorterBaseCDCHit), m_sortingCacheCDC(true),
  m_svdHitsName(svdHitsName), m_pxdHitsName(pxdHitsName),
  m_assumeSortedVXD(false),
  m_sorterBaseVXD(sorterBaseVXDHit),
  m_sortingCacheVXD(true)

{
  m_cdcHitIndicesPositive.reserve(1000);
  m_cdcHitIndicesNegative.reserve(1000);
  m_svdHitIndicesPositive.reserve(32);
  m_svdHitIndicesNegative.reserve(32);
  m_pxdHitIndicesPositive.reserve(8);
  m_pxdHitIndicesNegative.reserve(8);
}

void RecoTrack::setCDCHitIndices(const std::vector< std::pair < unsigned short, short> >& cdcHitIndices,
                                 const short pseudocharge)
{
  pseudocharge > 0 ?
  (m_cdcHitIndicesPositive = cdcHitIndices) :
  (m_cdcHitIndicesNegative = cdcHitIndices);
  m_sortingCacheCDC = false;
}

void RecoTrack::addCDCHitIndex(const std::pair< unsigned short, short> cdcHitIndex,
                               const short pseudoCharge)
{
  pseudoCharge > 0 ?
  (m_cdcHitIndicesPositive.push_back(cdcHitIndex)) :
  (m_cdcHitIndicesNegative.push_back(cdcHitIndex));
  m_sortingCacheCDC = false;
}

void RecoTrack::addCDCHitIndices(const std::vector< std::pair < unsigned short, short> >& cdcHitIndices,
                                 const short pseudoCharge)
{
  pseudoCharge > 0 ?
  m_cdcHitIndicesPositive.insert(m_cdcHitIndicesPositive.end(), cdcHitIndices.begin(), cdcHitIndices.end()) :
  m_cdcHitIndicesNegative.insert(m_cdcHitIndicesNegative.end(), cdcHitIndices.begin(), cdcHitIndices.end());
  m_sortingCacheCDC = false;
}

bool RecoTrack::hasCDCHit(const unsigned short hitIndex, const short rightLeft)
{
  return ((m_cdcHitIndicesNegative.end() !=
           std::find(m_cdcHitIndicesNegative.begin(), m_cdcHitIndicesNegative.end(), make_pair(hitIndex, rightLeft)))
          or
          (m_cdcHitIndicesPositive.end() !=
           std::find(m_cdcHitIndicesPositive.begin(), m_cdcHitIndicesPositive.end(), make_pair(hitIndex, rightLeft)))
         );
}

void RecoTrack::fillHitPatternCDC(const short pseudoCharge)
{
  StoreArray<CDCHit> cdcHits(m_cdcHitsName);
  HitPatternCDC      hitPatternCDC;
  auto& cdcHitIndices =
    pseudoCharge > 0 ? m_cdcHitIndicesPositive : m_cdcHitIndicesNegative;

  for (auto hitPair : cdcHitIndices) {
    // I need to initialize a WireID with the ID from the CDCHit to get the continuous layer ID.
    WireID wireID(cdcHits[hitPair.first]->getID());
    // Then I set the corresponding layer in the hit pattern.
    hitPatternCDC.setLayer(wireID.getICLayer());
  }
  m_hitPatternCDCInitializer = hitPatternCDC.getInteger();
}

void RecoTrack::fillHitPatternVXD(const short pseudoCharge)
{
  //Do something similar as for the CDC
}


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
