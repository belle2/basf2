#include <tracking/trackFinderOutputCombining/Lookups.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <framework/logging/LogSystem.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackFinderOutputCombining;

void SegmentLookUp::fillWith(std::vector<CDCRecoSegment2D>& segments)
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  // Calculate a lookup SuperLayerID -> Segments
  m_lookup.clear();
  m_lookup.resize(wireTopology.N_SUPERLAYERS);

  for (CDCRecoSegment2D& segment : segments) {
    ILayerType superlayerID = segment.getISuperLayer();
    m_lookup[superlayerID].push_back(new SegmentInformation(&segment));
    B2DEBUG(200, "Added new segment to segment lookup: " << segment.getTrajectory2D())
  }
}

void TrackLookUp::fillWith(std::vector<CDCTrack>& tracks)
{
  // Calculate a lookup for Tracks
  m_lookup.clear();
  m_lookup.reserve(tracks.size());

  // Calculate a lookup Track -> TrackInformation
  for (CDCTrack& trackCand : tracks) {
    TrackInformation* trackInformation = new TrackInformation(&trackCand);

    std::vector<double>& perpSList = trackInformation->getPerpSList();
    perpSList.reserve(trackCand.size());
    for (const CDCRecoHit3D& recoHit : trackCand) {
      perpSList.push_back(recoHit.getPerpS());
    }
    trackInformation->calcPerpS();
    m_lookup.push_back(trackInformation);
    B2DEBUG(200, "Added new track to track lookup: " << trackCand.getStartTrajectory3D().getTrajectory2D())
  }
}

void SegmentTrackCombiner::combine(BaseSegmentTrackChooser& segmentTrackChooser,
                                   BaseSegmentTrainFilter& segmentTrainFilter,
                                   BaseSegmentTrackFilter& segmentTrackFilter)
{
  B2DEBUG(100, "Starting combining work.");

  for (std::vector<SegmentInformation*>& segmentsList : m_segmentLookUp) {
    B2DEBUG(100, "Starting next superlayer.");

    // Search for all matching tracks for a given segment
    for (SegmentInformation* segmentInformation : segmentsList) {
      matchTracksToSegment(segmentInformation, segmentTrackChooser);
    }

    // Go through all tracks and delete the cases were we have more than one train/segment
    for (TrackInformation* trackInformation : m_trackLookUp) {
      // Try to find the longest trains of segments
      std::list<TrainOfSegments> trainsOfSegments;

      // TODO: Be careful with curlers here!!!!
      B2DEBUG(100, "Track matches to " << trackInformation->getMatches().size() << " segments before train creation.")
      createTrainsOfMatchedSegments(trainsOfSegments, trackInformation, segmentTrainFilter);

      if (trainsOfSegments.size() == 1) {
        // If there is only one matching segment/train, mark this as matched
        B2DEBUG(100, "Only one train/segment which matches! Good!")
        trackInformation->setGoodSegmentTrain(trainsOfSegments.front());
      } else if (trainsOfSegments.size() > 1) {
        // There are no good reasons why we should have more than one train/segment that matches. We have to find the best one.
        B2WARNING("Number of possible trains/segments still left: " << trainsOfSegments.size() <<
                  ". As these are too much we will only us the best one and delete the rest.");
        trackInformation->setGoodSegmentTrain(findBestFittingSegmentTrain(trainsOfSegments, trackInformation, segmentTrackFilter));
      }

      // Reset the matching segments lists
      trackInformation->clearMatches();
    }

    // Go through the tracks and see if we can combine the remaining segment-track combinations
    for (TrackInformation* trackInformation : m_trackLookUp) {
      B2DEBUG(100, "Looking for possible combinations..")
      const TrainOfSegments& goodTrain = trackInformation->getGoodSegmentTrain();
      if (goodTrain.size() > 0)
        tryToCombineSegmentTrainAndMatchedTracks(goodTrain);

      trackInformation->clearGoodSegmentTrain();
    }

    // Count the number of segments we have still left
    unsigned int notUsedSegments = std::count_if(segmentsList.begin(),
    segmentsList.end(), [](const SegmentInformation * segmentInformation) -> bool {
      if (segmentInformation->isUsedInTrack())
      {
        segmentInformation->getSegment()->getAutomatonCell().setTakenFlag();
        return false;
      } else {
        segmentInformation->getSegment()->getAutomatonCell().unsetTakenFlag();
        return true;
      }
    });

    if (notUsedSegments > 0) {
      B2WARNING("Still " << notUsedSegments << " not used in this superlayer.")
    }

  }
}

const SegmentTrackCombiner::TrainOfSegments& SegmentTrackCombiner::findBestFittingSegmentTrain(
  std::list<TrainOfSegments>& trainsOfSegments, TrackInformation* trackInformation, BaseSegmentTrackFilter& segmentTrackFilter)
{
  // We can easily delete all matches here as we have them in the list anyway
  trackInformation->clearMatches();

  TrainOfSegments* bestFittingTrain = nullptr;
  double bestProbability = -1;
  for (TrainOfSegments& trainOfSegments : trainsOfSegments) {
    for (SegmentInformation* segmentInformation : trainOfSegments) {
      std::vector<TrackInformation*>& matchedTracks = segmentInformation->getMatches();
      matchedTracks.erase(std::remove(matchedTracks.begin(), matchedTracks.end(), trackInformation), matchedTracks.end());
    }

    double currentProbability = testFitSegmentTrainToTrack(trainOfSegments, trackInformation);
    if (currentProbability > bestProbability) {
      bestFittingTrain = &trainOfSegments;
      bestProbability = currentProbability;
    }
  }

  TrainOfSegments& bestFittingSegmentTrain = *bestFittingTrain;

  // We have to readd the matches we want to keep
  for (SegmentInformation* segmentInformation : bestFittingSegmentTrain) {
    segmentInformation->addMatch(trackInformation);
  }
  return bestFittingSegmentTrain;
}

void SegmentTrackCombiner::makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments,
                                               const TrackInformation* trackInformation,
                                               BaseSegmentTrainFilter& segmentTrainFilter)
{
  const std::vector<SegmentInformation*>& matchedSegments = trackInformation->getMatches();

  trainsOfSegments.clear();
  trainsOfSegments.emplace_back();

  const CDCTrajectory2D& trajectory2D = trackInformation->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  for (SegmentInformation* segment : matchedSegments) {
    std::list<std::vector<SegmentInformation*>> innerSet;
    for (std::vector<SegmentInformation*> x : trainsOfSegments) {
      x.push_back(segment);
      std::sort(x.begin(), x.end(), [&trajectory2D](SegmentInformation * first, SegmentInformation * second) {
        return trajectory2D.calcPerpS(first->getSegment()->front().getRecoPos2D()) >
               trajectory2D.calcPerpS(second->getSegment()->front().getRecoPos2D());
      });
      if (not isNotACell(segmentTrainFilter(std::make_pair(x, trackInformation->getTrackCand()))))
        innerSet.push_back(x);
    }
    trainsOfSegments.insert(trainsOfSegments.end(), innerSet.begin(), innerSet.end());
  }
}

void SegmentTrackCombiner::addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTrack)
{
  const CDCTrajectory2D& trajectory2D = matchingTrack->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  segmentInformation->setUsedInTrack();
  for (const CDCRecoHit2D& recoHit : segmentInformation->getSegment()->items()) {
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
    matchingTrack->getTrackCand()->push_back(recoHit3D);
    matchingTrack->getPerpSList().push_back(
      trajectory2D.calcPerpS(recoHit.getRecoPos2D()));
  }
  matchingTrack->calcPerpS();
}

double SegmentTrackCombiner::testFitSegmentTrainToTrack(const TrainOfSegments& train, const TrackInformation* trackInformation)
{
  if (train.size() == 0) return -1;


  const CDCTrajectory2D& trajectory2D = trackInformation->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  CDCObservations2D observations;

  CDCTrack* trackCand = trackInformation->getTrackCand();

  bool isAxialSegment = train.front()->getSegment()->getStereoType() != AXIAL;

  for (const CDCRecoHit3D& recoHit : trackCand->items()) {
    if (isAxialSegment) {
      observations.append(recoHit.getWireHit().getRefPos2D());
    } else {
      double s = recoHit.getPerpS();
      double z = recoHit.getRecoZ();
      observations.append(s, z);
    }
  }

  for (SegmentInformation* segmentInformation : train) {
    for (const CDCRecoHit2D& recoHit : segmentInformation->getSegment()->items()) {
      if (isAxialSegment) {
        observations.append(recoHit.getRecoPos2D());
      } else {
        CDCRLWireHit rlWireHit(recoHit.getWireHit(), recoHit.getRLInfo());
        CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory2D);
        double s = recoHit3D.getPerpS();
        double z = recoHit3D.getRecoZ();
        observations.append(s, z);
      }
    }
  }

  if (isAxialSegment) {
    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    CDCTrajectory2D fittedTrajectory = fitter.fit(observations);
    return TMath::Prob(fittedTrajectory.getChi2(), fittedTrajectory.getNDF());
  } else {
    const CDCSZFitter& fitter = CDCSZFitter::getFitter();
    CDCTrajectorySZ fittedTrajectory = fitter.fit(observations);
    return TMath::Prob(fittedTrajectory.getChi2(), fittedTrajectory.getNDF());
  }
}

double SegmentTrackCombiner::testFitSegmentToTrack(SegmentInformation* segmentInformation,
                                                   const TrackInformation* trackInformation)
{
  TrainOfSegments temporaryTrain {segmentInformation};
  return testFitSegmentTrainToTrack(temporaryTrain, trackInformation);
}

void SegmentTrackCombiner::tryToCombineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments)
{
  B2DEBUG(100, "Trying to combine " << trainOfSegments.size() << " segment(s) with their track(s)...")
  for (SegmentInformation* segmentInformation : trainOfSegments) {

    std::vector<TrackInformation*>& matchingTracks = segmentInformation->getMatches();
    B2DEBUG(100, "Segment has " << matchingTracks.size() << " partners.")

    if (matchingTracks.size() == 0) {
      B2DEBUG(100, "Match was deleted.")
      return;
    }

    // Try to fiddle out which is the best one!
    double bestFitProb = 0;
    TrackInformation* bestMatch = nullptr;

    std::vector<TrackInformation*> matchesAboveTrack;
    matchesAboveTrack.reserve(matchingTracks.size() / 2);

    matchingTracks.erase(
      std::remove_if(matchingTracks.begin(), matchingTracks.end(), [&segmentInformation,
    &matchesAboveTrack](TrackInformation * possiblyMatch) -> bool {
      const CDCTrajectory2D& trajectory2D = possiblyMatch->getTrackCand()->getStartTrajectory3D().getTrajectory2D();
      double perpSFront = trajectory2D.calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
      double perpSBack = trajectory2D.calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());
      if (perpSFront > possiblyMatch->getMaxPerpS() or perpSBack < possiblyMatch->getMinPerpS())
      {
        B2DEBUG(120, "Segment is above or below track.")
        matchesAboveTrack.push_back(possiblyMatch);
        return true;
      } else {
        return false;
      }
    }), matchingTracks.end());

    if (matchingTracks.size() == 1) {
      bestMatch = matchingTracks[0];
      B2DEBUG(100, "Combining segment with track: " << bestMatch->getTrackCand())
    } else if (matchingTracks.size() > 1) {
      B2DEBUG(100, matchingTracks.size() << " are too many possible partners! We choose the best one:")

      for (TrackInformation* trackInformation : matchingTracks) {
        double fitProbability = testFitSegmentToTrack(segmentInformation, trackInformation);

        if (fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb)
      } else {
        B2DEBUG(100, "Found no segment with a good fit value in the track.")
        bestMatch = nullptr;
      }
    } else if (matchingTracks.size() == 0) {
      B2WARNING("None of the matches were in the track. Aborting. There are " << matchesAboveTrack.size() << " matches above the track.")

      // Try to fit them to the track
      double bestFitProb = 0;
      TrackInformation* bestMatch = nullptr;

      for (TrackInformation* trackInformation : matchesAboveTrack) {
        double fitProbability = testFitSegmentToTrack(segmentInformation, trackInformation);

        if (fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track above/below after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb)
      } else {
        B2DEBUG(100, "Found no segment with a good fit value above/below the track.")
        bestMatch = nullptr;
      }
    }

    if (bestMatch != nullptr)
      addSegmentToTrack(segmentInformation, bestMatch);
    matchingTracks.clear();
  }
}

void SegmentTrackCombiner::matchTracksToSegment(SegmentInformation* segmentInformation,
                                                BaseSegmentTrackChooser& segmentTrackChooser)
{
  for (TrackInformation* trackInformation : m_trackLookUp) {
    segmentTrackChooser.clear();
    if (isNotACell(segmentTrackChooser(std::make_pair(segmentInformation->getSegment(), trackInformation->getTrackCand())))) {
      B2DEBUG(110, "Found not matchable in " << segmentInformation->getSegment()->getISuperLayer())
    } else {
      B2DEBUG(110, "Found matchable in " << segmentInformation->getSegment()->getISuperLayer())
      trackInformation->addMatch(segmentInformation);
      segmentInformation->addMatch(trackInformation);
    }
  }
}

void SegmentTrackCombiner::createTrainsOfMatchedSegments(std::list<TrainOfSegments>& trainsOfSegments,
                                                         const TrackInformation* trackInformation,
                                                         BaseSegmentTrainFilter& segmentTrainFilter)
{
  const std::vector<SegmentInformation*>& matchedSegments = trackInformation->getMatches();
  if (matchedSegments.size() == 1) {
    trainsOfSegments.push_back(matchedSegments);
  } else if (matchedSegments.size() > 1) {
    makeAllCombinations(trainsOfSegments, trackInformation, segmentTrainFilter);
  }
}
