/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/NaiveCombinerModule.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/SegmentTrackCombiner.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(NaiveCombiner);

NaiveCombinerModule::NaiveCombinerModule() : TrackFinderCDCFromSegmentsModule()
{
  setDescription("Tries to combine the two outputs of two implementations of the track finder algorithm."
                 "Does so by just throwing everything together in one list (naive approach) or by using mc info (best approach)");

  addParam("useMCInformation",
           m_param_useMCInformation,
           "Uses the MC information from the MCTrackFinder and the MCMatcher to merge tracks.",
           false);
  addParam("useTakenFlagOfHits",
           m_param_useTakenFlagOfHits,
           "If yes (and not using MC information), copy only those hits of the segments to new tracks which are not already found by the Legendre track finder.",
           false);
}

void NaiveCombinerModule::generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments,
                                   std::vector<TrackFindingCDC::CDCTrack>& tracks)
{
  // if we do not use the mc information, we just throw all the tracks into one store array
  // if we use the mc information we add up all the track cands that are matched together
  if (not m_param_useMCInformation) {
    for (const CDCRecoSegment2D& segment : segments) {
      if (segment.getAutomatonCell().hasTakenFlag())
        continue;

      tracks.emplace_back();
      CDCTrack& newTrack = tracks.back();

      const CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
      CDCTrajectory3D trajectory(trajectory2D, CDCTrajectorySZ::basicAssumption());
      newTrack.setStartTrajectory3D(trajectory);

      SegmentTrackCombiner::addSegmentToTrack(segment, newTrack, m_param_useTakenFlagOfHits);
      if (newTrack.size() == 0)
        tracks.pop_back();
    }
  } else {
    CDCMCManager::getInstance().fill();

    const CDCMCSegmentLookUp& mcSegmentLookup = CDCMCSegmentLookUp::getInstance();
    const CDCMCHitLookUp& mcHitLookup =  CDCMCHitLookUp::getInstance();


    for (const CDCRecoSegment2D& segment : segments) {
      if (segment.getAutomatonCell().hasTakenFlag())
        continue;

      ITrackType segmentMCMatch = mcSegmentLookup.getMCTrackId(&segment);
      if (segmentMCMatch == INVALID_ITRACK)
        continue;

      bool hasPartner = false;

      for (CDCTrack& track : tracks) {
        double numberOfCorrectHits = 0;
        for (const CDCRecoHit3D& recoHit : track) {
          if (mcHitLookup.getMCTrackId(recoHit->getWireHit()->getHit()) == segmentMCMatch) {
            numberOfCorrectHits++;
          }
        }

        if (numberOfCorrectHits / track.size() > 0.5) {
          hasPartner = true;
          SegmentTrackCombiner::addSegmentToTrack(segment, track);
          break;
        }
      }

      if (not hasPartner) {
        tracks.emplace_back();
        CDCTrack& newTrack = tracks.back();

        const CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
        CDCTrajectory3D trajectory(trajectory2D, CDCTrajectorySZ::basicAssumption());
        newTrack.setStartTrajectory3D(trajectory);

        SegmentTrackCombiner::addSegmentToTrack(segment, newTrack);
        if (newTrack.size() == 0)
          tracks.pop_back();
      }
    }
  }
}

