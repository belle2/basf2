/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCMCTruthModule.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentFinderCDCMCTruth);

SegmentFinderCDCMCTruthModule::SegmentFinderCDCMCTruthModule() : SegmentFinderCDCBaseModule()
{
  setDescription("Generates segments from Monte Carlo information.");
  addParam("MinCDCHits", m_minCDCHits, "Minimum number of CDC hits needed to allow the created of a track candidate", 0);
}

void SegmentFinderCDCMCTruthModule::initialize()
{
  SegmentFinderCDCBaseModule::initialize();
  StoreArray <CDCSimHit>::required();
  StoreArray <MCParticle>::required();
}


void SegmentFinderCDCMCTruthModule::generateSegments(std::vector<CDCRecoSegment2D>& outputSegments)
{
  CDCMCManager::getInstance().fill();

  const CDCMCTrackStore& mcTrackStore = CDCMCTrackStore::getInstance();
  const CDCSimHitLookUp& simHitLookUp = CDCSimHitLookUp::getInstance();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const std::vector<CDCWireHit> wireHits = wireHitTopology.getWireHits();

  typedef Belle2::TrackFindingCDC::CDCMCTrackStore::CDCHitVector CDCHitVector;

  const std::map<ITrackType, std::vector<CDCHitVector>>& mcSegmentsByMCParticleIdx = mcTrackStore.getMCSegmentsByMCParticleIdx();
  for (const std::pair<ITrackType, std::vector<CDCHitVector>>& mcSegmentsAndMCParticleIdx : mcSegmentsByMCParticleIdx) {

    const std::vector<CDCHitVector>& mcSegments =  mcSegmentsAndMCParticleIdx.second;

    // Reserve double the space if both orientations shall be generated
    outputSegments.reserve(mcSegments.size());

    for (const CDCHitVector& mcSegment : mcSegments) {
      outputSegments.push_back(CDCRecoSegment2D());
      CDCRecoSegment2D& recoSegment2D = outputSegments.back();

      for (const CDCHit* ptrHit : mcSegment) {
        CDCRecoHit2D recoHit2D = simHitLookUp.getClosestPrimaryRecoHit2D(ptrHit, wireHits);
        recoSegment2D.push_back(recoHit2D);
      }
    }
  }
}
