/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCSegmentWorker.h"

#include <tracking/cdcLocalTracking/mclookup/CDCMCTrackStore.h>
#include <tracking/cdcLocalTracking/mclookup/CDCSimHitLookUp.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

std::vector< CDCRecoSegment2D >& MCSegmentWorker::generate(bool allowBackward)
{
  generate(m_segments2D, allowBackward);
  return m_segments2D;
}

void MCSegmentWorker::generate(std::vector< CDCRecoSegment2D >& outputSegments, bool allowBackward)
{

  m_segments2D.clear();
  outputSegments.clear();

  const CDCMCTrackStore& mcTrackStore = CDCMCTrackStore::getInstance();
  const CDCSimHitLookUp& simHitLookUp = CDCSimHitLookUp::getInstance();

  typedef Belle2::CDCLocalTracking::CDCMCTrackStore::CDCHitVector CDCHitVector;

  const std::map<ITrackType, std::vector<CDCHitVector>>& mcSegmentsByMCParticleIdx = mcTrackStore.getMCSegmentsByMCParticleIdx();
  for (const std::pair<ITrackType, std::vector<CDCHitVector>>& mcSegmentsAndMCParticleIdx : mcSegmentsByMCParticleIdx) {

    const std::vector<CDCHitVector>& mcSegments =  mcSegmentsAndMCParticleIdx.second;
    for (const CDCHitVector & mcSegment : mcSegments) {

      m_segments2D.push_back(CDCRecoSegment2D());
      CDCRecoSegment2D& recoSegment2D = m_segments2D.back();

      for (const CDCHit * ptrHit : mcSegment) {
        CDCRecoHit2D recoHit2D = simHitLookUp.getClosestPrimaryRecoHit2D(ptrHit);
        recoSegment2D.push_back(recoHit2D);
      }

      if (allowBackward) {
        m_segments2D.reserve(m_segments2D.size() + 1);
        m_segments2D.push_back(m_segments2D.back().reversed());
        // ^ Save because we reserved the memory beforehand.
      }

    }

  }

  if (m_copyToDataStoreForDebug) {
    copyToDataStoreForDebug();
  }

  outputSegments.swap(m_segments2D);
}



void MCSegmentWorker::copyToDataStoreForDebug() const
{

  // IO for monitoring in python
#ifdef CDCLOCALTRACKING_USE_ROOT

  // IO selected segments without tangents
  B2DEBUG(100, "  Creating the StoreArray for the generated segments");
  StoreArray < CDCRecoSegment2D > storedSegments2D;
  storedSegments2D.create();
  B2DEBUG(100, "  Copying the selected CDCRecoSegment2Ds to the StoreArray");
  for (const CDCRecoSegment2D & segment2D : m_segments2D) {
    storedSegments2D.appendNew(segment2D);
  }
  B2DEBUG(100, "  Created " << storedSegments2D.getEntries()  <<
          " selected CDCRecoSegment2D");
#endif

}
