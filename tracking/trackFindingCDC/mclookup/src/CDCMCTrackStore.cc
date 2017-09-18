/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>

#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCMap.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <tracking/trackFindingCDC/ca/WithAutomatonCell.h>
#include <tracking/trackFindingCDC/ca/Clusterizer.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <memory>

using namespace Belle2;
using namespace TrackFindingCDC;

const CDCMCTrackStore& CDCMCTrackStore::getInstance()
{
  return CDCMCManager::getMCTrackStore();
}


void CDCMCTrackStore::clear()
{

  B2DEBUG(200, "In CDCMCTrackStore::clear()");

  m_ptrMCMap = nullptr;

  m_mcTracksByMCParticleIdx.clear();
  m_mcSegmentsByMCParticleIdx.clear();

  m_inTrackIds.clear();
  m_inTrackSegmentIds.clear();
  m_nPassedSuperLayers.clear();
  m_nLoops.clear();

}



void CDCMCTrackStore::fill(const CDCMCMap* ptrMCMap, const CDCSimHitLookUp* ptrSimHitLookUp)
{

  B2DEBUG(200, "In CDCMCTrackStore::fill()");
  clear();

  m_ptrMCMap = ptrMCMap;
  m_ptrSimHitLookUp = ptrSimHitLookUp;

  // Put the right hits into the right track
  fillMCTracks();

  // Split the tracks into segments
  fillMCSegments();

  // Assign the reverse mapping from CDCHits to position in track
  fillInTrackId();

  // Assigne the reverse mapping from CDCHits to segment ids
  fillInTrackSegmentId();

  // Assigne the reverse mapping from CDCHits to the number of already traversed superlayers
  fillNLoopsAndNPassedSuperLayers();

  B2DEBUG(100, "m_mcTracksByMCParticleIdx.size(): " << m_mcTracksByMCParticleIdx.size());
  B2DEBUG(100, "m_mcSegmentsByMCParticleIdx.size(): " << m_mcSegmentsByMCParticleIdx.size());

  B2DEBUG(100, "m_inTrackIds.size(): " << m_inTrackIds.size());
  B2DEBUG(100, "m_inTrackSegmentIds.size() " << m_inTrackSegmentIds.size());
  B2DEBUG(100, "m_nPassedSuperLayers.size(): " << m_nPassedSuperLayers.size());
  B2DEBUG(100, "m_nLoops.size(): " << m_nLoops.size());

}


void CDCMCTrackStore::fillMCTracks()
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot create tracks");
    return;
  }

  const CDCMCMap& mcMap = *m_ptrMCMap;

  for (const auto& relation : mcMap.getHitsByMCParticle()) {

    const MCParticle* ptrMCParticle = std::get<const MCParticle* const>(relation);
    const CDCHit* ptrHit = std::get<const CDCHit*>(relation);

    if (not mcMap.isBackground(ptrHit) and ptrMCParticle) {

      ITrackType mcParticleIdx = ptrMCParticle->getArrayIndex();
      //Append hit to its own track
      m_mcTracksByMCParticleIdx[mcParticleIdx].push_back(ptrHit);
    }
  }


  //Sort the tracks along the time of flight
  for (std::pair<const ITrackType, CDCHitVector>& mcTrackAndMCParticleIdx : m_mcTracksByMCParticleIdx) {

    //int mcParticleIdx = mcTrackAndMCParticleIdx.first;
    CDCHitVector& mcTrack = mcTrackAndMCParticleIdx.second;
    arrangeMCTrack(mcTrack);

  }

}

void CDCMCTrackStore::fillMCSegments()
{
  for (std::pair<const ITrackType, CDCHitVector>& mcTrackAndMCParticleIdx : m_mcTracksByMCParticleIdx) {

    ITrackType mcParticleIdx = mcTrackAndMCParticleIdx.first;
    CDCHitVector& mcTrack = mcTrackAndMCParticleIdx.second;

    if (mcTrack.empty()) continue;
    std::vector<CDCHitVector>& mcSegments = m_mcSegmentsByMCParticleIdx[mcParticleIdx];
    mcSegments.clear();

    // Split track into runs in the same superlayer
    auto superLayerRanges = adjacent_groupby(mcTrack.begin(), mcTrack.end(), [](const CDCHit * hit) {
      return hit->getISuperLayer();
    });

    std::vector<CDCHitVector> superLayerRuns;
    for (const auto& superLayerRange : superLayerRanges) {
      superLayerRuns.push_back({superLayerRange.begin(), superLayerRange.end()});
    }

    std::vector<std::vector<CDCHitVector>::iterator> smallSuperLayerRuns;
    for (auto itSuperLayerRun = superLayerRuns.begin();
         itSuperLayerRun != superLayerRuns.end();
         ++itSuperLayerRun) {
      if (itSuperLayerRun->size() < 3) smallSuperLayerRuns.push_back(itSuperLayerRun);
    }

    // Merge small run to an adjacent run
    for (auto itSuperLayerRun : smallSuperLayerRuns) {
      ISuperLayer iSL = itSuperLayerRun->front()->getISuperLayer();

      // Look in both directions to adopt the hits in this small runs
      auto itSuperLayerRunBefore = superLayerRuns.end();
      int hitDistanceBefore = INT_MAX;
      if (std::distance(superLayerRuns.begin(), itSuperLayerRun) >= 2) {
        itSuperLayerRunBefore = itSuperLayerRun - 2;
        if (itSuperLayerRunBefore->front()->getISuperLayer() == iSL) {
          hitDistanceBefore = (itSuperLayerRunBefore - 1)->size();
        } else {
          itSuperLayerRunBefore = superLayerRuns.end();
        }
      }

      auto itSuperLayerRunAfter = superLayerRuns.end();
      int hitDistanceAfter = INT_MAX;
      if (std::distance(itSuperLayerRun, superLayerRuns.end()) > 2) {
        itSuperLayerRunAfter = itSuperLayerRun + 2;
        if (itSuperLayerRunAfter->front()->getISuperLayer() == iSL) {
          hitDistanceAfter = (itSuperLayerRunAfter + 1)->size();
        } else {
          itSuperLayerRunAfter = superLayerRuns.end();
        }
      }

      auto itMergeSuperLayerRun = superLayerRuns.end();
      bool mergeBefore = false;
      if (hitDistanceBefore < hitDistanceAfter) {
        itMergeSuperLayerRun = itSuperLayerRunBefore;
        mergeBefore = true;
      } else {
        itMergeSuperLayerRun = itSuperLayerRunAfter;
        mergeBefore = false;
      }

      if (itMergeSuperLayerRun == superLayerRuns.end()) continue;
      else if (mergeBefore) {
        itMergeSuperLayerRun->insert(itMergeSuperLayerRun->end(), itSuperLayerRun->begin(), itSuperLayerRun->end());
        itSuperLayerRun->clear();
      } else {
        itMergeSuperLayerRun->insert(itMergeSuperLayerRun->begin(), itSuperLayerRun->begin(), itSuperLayerRun->end());
        itSuperLayerRun->clear();
      }
    }

    // Remove empty small runs
    erase_remove_if(superLayerRuns, Size() == 0u);

    // Concat the runs that are now in the same superlayer
    auto mergeSameSuperLayer = [](CDCHitVector & lhs, CDCHitVector & rhs) {
      if (lhs.empty() or rhs.empty()) return true;
      if (lhs.front()->getISuperLayer() != rhs.front()->getISuperLayer()) return false;
      lhs.insert(lhs.end(), rhs.begin(), rhs.end());
      rhs.clear();
      return true;
    };
    erase_unique(superLayerRuns, mergeSameSuperLayer);

    // Now analyse the runs in turn and break them in the connected segments
    for (const CDCHitVector& superLayerRun : superLayerRuns) {
      const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
      auto areNeighbors = [&wireTopology](const CDCHit * lhs, const CDCHit * rhs) {
        WireID lhsWireID(lhs->getISuperLayer(), lhs->getILayer(), lhs->getIWire());
        WireID rhsWireID(rhs->getISuperLayer(), rhs->getILayer(), rhs->getIWire());

        return (wireTopology.arePrimaryNeighbors(lhsWireID, rhsWireID) or
                wireTopology.areSeconaryNeighbors(lhsWireID, rhsWireID) or
                lhsWireID == rhsWireID);
      };

      auto segmentRanges = unique_ranges(superLayerRun.begin(), superLayerRun.end(), areNeighbors);

      for (const ConstVectorRange<const CDCHit*> segmentRange : segmentRanges) {
        mcSegments.emplace_back(segmentRange.begin(), segmentRange.end());
      }
    } // end for superLayerRuns

    // Lets sort them along for the time of flight.
    for (CDCHitVector& mcSegment : mcSegments) {
      arrangeMCTrack(mcSegment);
    }
  } // End for mc track
}

void CDCMCTrackStore::arrangeMCTrack(CDCHitVector& mcTrack) const
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot sort track");
    return;
  }

  const CDCSimHitLookUp& simHitLookUp = *m_ptrSimHitLookUp;

  std::stable_sort(mcTrack.begin(), mcTrack.end(),
  [&simHitLookUp](const CDCHit * ptrHit, const CDCHit * ptrOtherHit) -> bool {

    const CDCSimHit* ptrSimHit = simHitLookUp.getClosestPrimarySimHit(ptrHit);
    const CDCSimHit* ptrOtherSimHit = simHitLookUp.getClosestPrimarySimHit(ptrOtherHit);

    if (not ptrSimHit)
    {
      B2FATAL("No CDCSimHit for CDCHit");
    }

    if (not ptrOtherSimHit)
    {
      B2FATAL("No CDCSimHit for CDCHit");
    }

    double secondaryFlightTime =  ptrSimHit->getFlightTime();
    double otherSecondaryFlightTime =  ptrOtherSimHit->getFlightTime();

    /// Sort with NaN as high.
    return (secondaryFlightTime < std::fmin(INFINITY, otherSecondaryFlightTime));
  });

}






void CDCMCTrackStore::fillInTrackId()
{

  for (const std::pair<ITrackType, CDCHitVector>& mcTrackAndMCParticleIdx : getMCTracksByMCParticleIdx()) {

    const CDCHitVector& mcTrack = mcTrackAndMCParticleIdx.second;

    //Fill the in track ids
    int iHit = -1;
    for (const CDCHit* ptrHit : mcTrack) {
      ++iHit;
      m_inTrackIds[ptrHit] = iHit;
    }
  }

}

void CDCMCTrackStore::fillInTrackSegmentId()
{
  for (const std::pair<ITrackType, std::vector<CDCHitVector> >& mcSegmentsAndMCParticleIdx : getMCSegmentsByMCParticleIdx()) {
    const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;

    int iSegment = -1;
    for (const CDCHitVector&   mcSegment : mcSegments) {
      ++iSegment;
      for (const CDCHit* ptrHit : mcSegment) {

        m_inTrackSegmentIds[ptrHit] = iSegment;
      }
    }
  }

}

void CDCMCTrackStore::fillNLoopsAndNPassedSuperLayers()
{

  for (const std::pair<ITrackType, std::vector<CDCHitVector> >& mcSegmentsAndMCParticleIdx : getMCSegmentsByMCParticleIdx()) {
    const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;

    const CDCHitVector* ptrLastMCSegment = nullptr;
    int nPassedSuperLayers = 0;
    int nLoops = 0;

    for (const CDCHitVector& mcSegment : mcSegments) {
      if (ptrLastMCSegment and changedSuperLayer(*ptrLastMCSegment, mcSegment)) {
        ++nPassedSuperLayers;

        // Increase the superlayer number if the track leaves the CDC for the inner volume.
        // Feel free to do something smarter here.
        if (ptrLastMCSegment->front()->getISuperLayer() == 0 and
            mcSegment.front()->getISuperLayer() == 0) {
          ++nLoops;
        }
      }

      for (const CDCHit* ptrHit : mcSegment) {
        m_nPassedSuperLayers[ptrHit] = nPassedSuperLayers;
        m_nLoops[ptrHit] = nLoops;
      }

      ptrLastMCSegment = &mcSegment;

    }
  }
}

bool CDCMCTrackStore::changedSuperLayer(const CDCHitVector& mcSegment, const CDCHitVector& nextMCSegment) const
{
  const CDCSimHitLookUp& simHitLookUp = *m_ptrSimHitLookUp;
  const CDCHit* ptrHit = mcSegment.front();
  const CDCHit* ptrNextHit = nextMCSegment.front();

  assert(ptrHit);
  assert(ptrNextHit);

  const CDCHit& hit = *ptrHit;
  const CDCHit& nextHit = *ptrNextHit;

  if (hit.getISuperLayer() != nextHit.getISuperLayer()) {
    return true;
  } else if (hit.getISuperLayer() == 0) {
    const CDCSimHit* ptrSimHit = simHitLookUp.getClosestPrimarySimHit(ptrHit);
    const CDCSimHit* ptrNextSimHit = simHitLookUp.getClosestPrimarySimHit(ptrNextHit);

    Vector3D pos(ptrSimHit->getPosTrack());
    Vector3D mom(ptrSimHit->getMomentum());
    Vector3D nextMom(ptrNextSimHit->getMomentum());
    Vector3D nextPos(ptrNextSimHit->getPosTrack());

    if (pos.dotXY(nextPos) < 0) return true;
    if ((nextPos - pos).dotXY(nextMom) < 0) return true;
    if ((nextPos - pos).dotXY(mom) < 0) return true;

    // TODO introduce a smarter check here
    return false;
  } else {
    return false;
  }
}


Index CDCMCTrackStore::getInTrackId(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_inTrackIds.find(ptrHit);
  return itFoundHit == m_inTrackIds.end() ? c_InvalidIndex : itFoundHit->second;

}



Index CDCMCTrackStore::getInTrackSegmentId(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_inTrackSegmentIds.find(ptrHit);
  return itFoundHit == m_inTrackSegmentIds.end() ? c_InvalidIndex : itFoundHit->second;

}


Index CDCMCTrackStore::getNPassedSuperLayers(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_nPassedSuperLayers.find(ptrHit);
  return itFoundHit == m_nPassedSuperLayers.end() ? c_InvalidIndex : itFoundHit->second;

}

Index CDCMCTrackStore::getNLoops(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_nLoops.find(ptrHit);
  return itFoundHit == m_nLoops.end() ? c_InvalidIndex : itFoundHit->second;

}
