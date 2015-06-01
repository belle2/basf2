/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/NaiveCombinerModule.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(NaiveCombiner);

NaiveCombinerModule::NaiveCombinerModule() : TrackFinderCDCFromSegmentsModule()
{
  setDescription("Tries to combine the two outputs of two implementations of the track finder algorithm.");

  addParam("MCTrackCands",
           m_param_mcTrackCands,
           "When UseMCInformation is enabled, you need to set the MCTrackCands here.",
           std::string("MCTrackCands"));

  addParam("UseMCInformation",
           m_param_useMCInformation,
           "Uses the MC information from the MCTrackFinder and the MCMatcher to merge tracks.",
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

      SegmentTrackCombiner::addSegmentToTrack(segment, newTrack);
      if (newTrack.size() == 0)
        tracks.pop_back();
    }
  } else {
    B2FATAL("Not implemented")
  }
}

