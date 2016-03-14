#include <tracking/trackFindingCDC/trackFinderOutputCombining/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SegmentTrackCombiner::clearAndRecover()
{
  for (const std::vector<SegmentInformation*>& segments : m_segmentLookUp) {
    for (SegmentInformation* segmentInformation : segments) {
      CDCRecoSegment2D* segment = segmentInformation->getSegment();
      if (segment->getAutomatonCell().hasMaskedFlag()) {
        segment->getAutomatonCell().unsetTakenFlag();
        segment->getAutomatonCell().unsetMaskedFlag();
      }
    }
  }

  m_trackLookUp.clear();
  m_segmentLookUp.clear();
}

void SegmentTrackCombiner::match(BaseSegmentTrackFilter& segmentTrackChooserFirstStep)
{
  // Mark the segments which are fully found by the legendre track finder as taken
  for (const std::vector<SegmentInformation*>& segments : m_segmentLookUp) {
    for (SegmentInformation* segment : segments) {
      const bool isFullyTaken = segment->getSegment()->isFullyTaken();

      if (isFullyTaken) {
        // Ensure that all hits belong to the same track!
        std::set<TrackInformation*> tracksWithHitsInCommon;
        for (const CDCRecoHit2D& recoHit : * (segment->getSegment())) {
          TrackInformation* trackWithHit = m_trackLookUp.findTrackForHit(recoHit);
          if (trackWithHit != nullptr) {
            tracksWithHitsInCommon.insert(trackWithHit);
          }
        }

        if (tracksWithHitsInCommon.size() == 1) {
          segment->getSegment()->getAutomatonCell().setTakenFlag();
          for (const CDCRecoHit2D& recoHit : * (segment->getSegment())) {
            TrackInformation* trackWithHit = m_trackLookUp.findTrackForHit(recoHit);
            if (trackWithHit != nullptr) {
              trackWithHit->getTrackCand()->setHasMatchingSegment();
              break;
            }
          }
        }
      }
    }
  }

  // prepare lookup
  for (TrackInformation* track : m_trackLookUp) {
    for (const CDCRecoHit3D& recoHit : * (track->getTrackCand())) {
      SegmentInformation* matchingSegment = m_segmentLookUp.findSegmentForHit(recoHit);

      if (matchingSegment == nullptr) {
        continue;
      }

      if (matchingSegment->isAlreadyTaken()) {
        continue;
      }

      // Check if we did not already have the track in the list
      const SegmentInformation::ListOfMatchCandidates& currentlyMatched = matchingSegment->getMatches();
      if (std::find_if(currentlyMatched.begin(),
      currentlyMatched.end(), [&track](const std::pair<TrackInformation*, double>& pair) -> bool {
      return pair.first == track;
    }) != currentlyMatched.end()) {
        continue;
      }

      // Call the filter and add the match
      double filterResult = segmentTrackChooserFirstStep(std::make_pair(matchingSegment->getSegment(), track->getTrackCand()));
      if (not isNotACell(filterResult)) {
        matchingSegment->addMatch(track, filterResult);
      }
    }
  }

  for (const std::vector<SegmentInformation*>& segments : m_segmentLookUp) {
    for (SegmentInformation* segment : segments) {
      if (segment->isAlreadyTaken())
        continue;

      const SegmentInformation::ListOfMatchCandidates& matchingTracks = segment->getMatches();

      if (matchingTracks.size() == 0)
        continue;

      TrackInformation* bestMatch = segment->getBestMatch();

      if (matchingTracks.size() > 1) {
        // Delete the hits from the other track(s)
        for (const std::pair<TrackInformation*, double>& matchingPair : matchingTracks) {
          TrackInformation* notBestTrack = matchingPair.first;

          if (notBestTrack == bestMatch) {
            continue;
          }

          CDCTrack* cdcTrack = notBestTrack->getTrackCand();

          cdcTrack->erase(std::remove_if(cdcTrack->begin(), cdcTrack->end(), [this, &segment](const CDCRecoHit3D & recoHit) -> bool {
            if (m_segmentLookUp.findSegmentForHit(recoHit) == segment)
            {
              recoHit->getWireHit().getAutomatonCell().unsetTakenFlag();
              return true;
            } else {
              return false;
            }
          }), cdcTrack->end());
        }
      }

      // Add the segment to the track with the highest probability to match
      SegmentTrackCombiner::addSegmentToTrack(segment, bestMatch);
    }
  }
}

void SegmentTrackCombiner::filterSegments(BaseBackgroundSegmentsFilter& backgroundSegmentsFilter)
{
  for (const std::vector<SegmentInformation*>& segments : m_segmentLookUp) {
    for (SegmentInformation* segment : segments) {
      if (segment->isAlreadyTaken())
        continue;
      Weight filterResult = backgroundSegmentsFilter(segment->getSegment());
      if (not isNotACell(filterResult)) {
        segment->getSegment()->getAutomatonCell().setTakenFlag();
        segment->getSegment()->getAutomatonCell().setBackgroundFlag();
      }
    }
  }
}

void SegmentTrackCombiner::filterOutNewSegments(BaseNewSegmentsFilter& newSegmentsFilter)
{
  for (const std::vector<SegmentInformation*>& segments : m_segmentLookUp) {
    for (SegmentInformation* segment : segments) {
      if (segment->isAlreadyTaken())
        continue;
      Weight filterResult = newSegmentsFilter(segment->getSegment());
      if (not isNotACell(filterResult)) {
        segment->getSegment()->getAutomatonCell().setTakenFlag();
        segment->getSegment()->getAutomatonCell().setMaskedFlag();
      }
    }
  }
}


void SegmentTrackCombiner::combine(BaseSegmentTrackFilter& segmentTrackFilterSecondStep,
                                   BaseSegmentTrainFilter& segmentTrainFilter,
                                   BaseSegmentInformationListTrackFilter& segmentTrackFilter)
{
  B2DEBUG(100, "Starting combining work.");

  for (std::vector<SegmentInformation*>& segmentsList : m_segmentLookUp) {
    B2DEBUG(100, "Starting next superlayer.");

    // Search for all matching tracks for a given segment
    for (SegmentInformation* segmentInformation : segmentsList) {
      if (segmentInformation->isAlreadyTaken()) {
        continue;
      }
      matchTracksToSegment(segmentInformation, segmentTrackFilterSecondStep);

      TrackInformation* bestMatch = segmentInformation->getBestMatch();
      if (bestMatch != nullptr) {
        SegmentTrackCombiner::addSegmentToTrack(segmentInformation, bestMatch);
        segmentInformation->clearMatches();
      }
    }

    // From here on the code is unused!

    // Go through all tracks and delete the cases were we have more than one train/segment
    for (TrackInformation* trackInformation : m_trackLookUp) {
      // Try to find the longest trains of segments
      std::list<TrainOfSegments> trainsOfSegments;

      // TODO: Be careful with curlers here!!!!
      B2DEBUG(100, "Track matches to " << trackInformation->getMatches().size() << " segments before train creation.");
      createTrainsOfMatchedSegments(trainsOfSegments, trackInformation, segmentTrainFilter);

      if (trainsOfSegments.size() == 1) {
        // If there is only one matching segment/train, mark this as matched
        B2DEBUG(100, "Only one train/segment which matches! Good!");
        trackInformation->setGoodSegmentTrain(trainsOfSegments.front());
      } else if (trainsOfSegments.size() > 1) {
        // There are no good reasons why we should have more than one train/segment that matches. We have to find the best one.
        B2DEBUG(100, "Number of possible trains/segments still left: " << trainsOfSegments.size() <<
                ". As these are too much we will only us the best one and delete the rest.");
        const TrainOfSegments* bestFittingTrain = findBestFittingSegmentTrain(trainsOfSegments, trackInformation, segmentTrackFilter);
        if (bestFittingTrain != nullptr)
          trackInformation->setGoodSegmentTrain(*bestFittingTrain);
      }

      // Reset the matching segments lists
      trackInformation->clearMatches();
    }

    // Go through the tracks and see if we can combine the remaining segment-track combinations
    for (TrackInformation* trackInformation : m_trackLookUp) {
      B2DEBUG(100, "Looking for possible combinations..");
      const TrainOfSegments& goodTrain = trackInformation->getGoodSegmentTrain();
      if (goodTrain.size() > 0)
        tryToCombineSegmentTrainAndMatchedTracks(goodTrain, segmentTrackFilter);

      trackInformation->clearGoodSegmentTrain();
    }
  }
}

const SegmentTrackCombiner::TrainOfSegments* SegmentTrackCombiner::findBestFittingSegmentTrain(
  std::list<TrainOfSegments>& trainsOfSegments, TrackInformation* trackInformation,
  BaseSegmentInformationListTrackFilter& segmentTrackFilter)
{
  // We can easily delete all matches here as we have them in the list anyway
  trackInformation->clearMatches();

  TrainOfSegments* bestFittingTrain = nullptr;
  double bestProbability = -1;
  for (TrainOfSegments& trainOfSegments : trainsOfSegments) {
    if (trainOfSegments.size() > 0) {
      for (SegmentInformation* segmentInformation : trainOfSegments) {
        std::vector<std::pair<TrackInformation*, double>>& matchedTracks = segmentInformation->getMatches();
        matchedTracks.erase(std::remove_if(matchedTracks.begin(),
        matchedTracks.end(), [&trackInformation](const std::pair<TrackInformation*, double>& pair) -> bool {
          return pair.first == trackInformation;
        }), matchedTracks.end());
      }

      CellWeight currentProbability = segmentTrackFilter(std::make_pair(trainOfSegments, trackInformation->getTrackCand()));
      if (not isNotACell(currentProbability) and currentProbability > bestProbability) {
        bestFittingTrain = &trainOfSegments;
        bestProbability = currentProbability;
      }
    }
  }

  if (bestFittingTrain != nullptr) {
    // We have to readd the matches we want to keep
    for (SegmentInformation* segmentInformation : *bestFittingTrain) {
      segmentInformation->addMatch(trackInformation, bestProbability);
    }

    return bestFittingTrain;
  } else {
    return nullptr;
  }
}

void SegmentTrackCombiner::clearSmallerCombinations(std::list<TrainOfSegments>& trainsOfSegments)
{
  // Sort by size descending (biggest at the front)
  trainsOfSegments.sort([](const TrainOfSegments & first, const TrainOfSegments & second) {
    return first.size() > second.size();
  });

  // Can not used a c++-11 range based for loop here, as I edit the container!
  for (auto testTrain = trainsOfSegments.begin(); testTrain != trainsOfSegments.end(); ++testTrain) {
    trainsOfSegments.erase(std::remove_if(trainsOfSegments.begin(),
    trainsOfSegments.end(), [&testTrain](const TrainOfSegments & train) -> bool {
      if (train.size() >= testTrain->size())
        return false;

      bool oneIsNotFound = false;
      for (const SegmentInformation* segmentInformation : train)
      {
        if (std::find(testTrain->begin(), testTrain->end(), segmentInformation) == train.end()) {
          oneIsNotFound = true;
          break;
        }
      }

      return not oneIsNotFound;
    }), trainsOfSegments.end());
  }
}

void SegmentTrackCombiner::makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments,
                                               const TrackInformation* trackInformation,
                                               BaseSegmentTrainFilter& segmentTrainFilter)
{
  const std::vector<std::pair<SegmentInformation*, double>>& matchedSegments = trackInformation->getMatches();

  trainsOfSegments.clear();
  trainsOfSegments.emplace_back();

  const CDCTrajectory2D& trajectory2D = trackInformation->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  for (const std::pair<SegmentInformation*, double>& pair : matchedSegments) {
    SegmentInformation* segment = pair.first;
    std::list<std::vector<SegmentInformation*>> innerSet;
    for (std::vector<SegmentInformation*> x : trainsOfSegments) {
      x.push_back(segment);
      std::sort(x.begin(), x.end(), [&trajectory2D](SegmentInformation * first, SegmentInformation * second) {
        return trajectory2D.calcArcLength2D(first->getSegment()->front().getRecoPos2D()) >
               trajectory2D.calcArcLength2D(second->getSegment()->front().getRecoPos2D());
      });
      Weight weight = segmentTrainFilter(std::make_pair(x, trackInformation->getTrackCand()));
      if (not isNotACell(weight)) {
        innerSet.push_back(x);
      }
    }
    trainsOfSegments.insert(trainsOfSegments.end(), innerSet.begin(), innerSet.end());
    if (trainsOfSegments.size() > 1000) {
      B2ERROR("Too much trains: " << trainsOfSegments.size() << "!");
      trainsOfSegments.clear();
      return;
    }
  }

  // Delete the first one: it is empty by definition
  trainsOfSegments.pop_front();

  // Delete all trains which are also found as a bigger one
  clearSmallerCombinations(trainsOfSegments);
}

void SegmentTrackCombiner::addSegmentToTrack(const CDCRecoSegment2D& segment, CDCTrack& track, const bool useTakenFlagOfHits)
{
  if (segment.getAutomatonCell().hasTakenFlag())
    return;

  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  for (const CDCRecoHit2D& recoHit : segment) {
    if (recoHit.getWireHit().getAutomatonCell().hasTakenFlag() and useTakenFlagOfHits)
      continue;
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
    track.push_back(recoHit3D);
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
  }

  track.setHasMatchingSegment();
  segment.getAutomatonCell().setTakenFlag();
}


void SegmentTrackCombiner::addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTrack)
{
  addSegmentToTrack(*(segmentInformation->getSegment()), *(matchingTrack->getTrackCand()));

  for (const CDCRecoHit3D& recoHit : * (matchingTrack->getTrackCand())) {
    matchingTrack->getArcLength2DList().push_back(recoHit.getArcLength2D());
  }
  matchingTrack->calcArcLength2D();
}


void SegmentTrackCombiner::tryToCombineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments,
    TrackFindingCDC::BaseSegmentInformationListTrackFilter& segmentTrackFilter)
{
  B2DEBUG(100, "Trying to combine " << trainOfSegments.size() << " segment(s) with their track(s)...");
  for (SegmentInformation* segmentInformation : trainOfSegments) {

    std::vector<std::pair<TrackInformation*, double>>& matchingTracks = segmentInformation->getMatches();
    B2DEBUG(100, "Segment has " << matchingTracks.size() << " partners.");

    if (matchingTracks.size() == 0) {
      B2DEBUG(100, "Match was deleted.");
      return;
    }

    // Try to fiddle out which is the best one!
    double bestFitProb = 0;
    TrackInformation* bestMatch = nullptr;

    std::vector<TrackInformation*> matchesAboveTrack;
    matchesAboveTrack.reserve(matchingTracks.size() / 2);

    matchingTracks.erase(
      std::remove_if(matchingTracks.begin(), matchingTracks.end(), [&segmentInformation,
    &matchesAboveTrack](const std::pair<TrackInformation*, double>& pair) -> bool {
      TrackInformation* possiblyMatch = pair.first;
      const CDCTrajectory2D& trajectory2D = possiblyMatch->getTrackCand()->getStartTrajectory3D().getTrajectory2D();
      double perpSFront = trajectory2D.calcArcLength2D(segmentInformation->getSegment()->front().getRecoPos2D());
      double perpSBack = trajectory2D.calcArcLength2D(segmentInformation->getSegment()->back().getRecoPos2D());
      if (perpSFront > possiblyMatch->getMaxArcLength2D() or perpSBack < possiblyMatch->getMinArcLength2D())
      {
        B2DEBUG(120, "Segment is above or below track.");
        matchesAboveTrack.push_back(possiblyMatch);
        return true;
      } else {
        return false;
      }
    }), matchingTracks.end());

    if (matchingTracks.size() == 1) {
      bestMatch = matchingTracks[0].first;
      B2DEBUG(100, "Combining segment with track: " << bestMatch->getTrackCand());
    } else if (matchingTracks.size() > 1) {
      B2DEBUG(100, matchingTracks.size() << " are too many possible partners! We choose the best one:");

      for (const std::pair<TrackInformation*, double>& pair : matchingTracks) {
        TrackInformation* trackInformation = pair.first;
        std::vector<SegmentInformation*> temporaryTrain = {segmentInformation};
        CellWeight fitProbability = segmentTrackFilter(std::make_pair(temporaryTrain, trackInformation->getTrackCand()));

        if (not isNotACell(fitProbability) and fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb);
      } else {
        B2DEBUG(100, "Found no segment with a good fit value in the track.");
        bestMatch = nullptr;
      }
    } else if (matchingTracks.size() == 0) {
      B2DEBUG(100, "None of the matches were in the track. Aborting. There are " << matchesAboveTrack.size() <<
              " matches above the track.");

      // Try to fit them to the track
      double bestFitProb = 0;
      TrackInformation* bestMatch = nullptr;

      for (TrackInformation* trackInformation : matchesAboveTrack) {
        std::vector<SegmentInformation*> temporaryTrain = {segmentInformation};
        CellWeight fitProbability = segmentTrackFilter(std::make_pair(temporaryTrain, trackInformation->getTrackCand()));

        if (not isNotACell(fitProbability) and fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track above/below after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb);
      } else {
        B2DEBUG(100, "Found no segment with a good fit value above/below the track.");
        bestMatch = nullptr;
      }
    }

    if (bestMatch != nullptr)
      addSegmentToTrack(segmentInformation, bestMatch);
    matchingTracks.clear();
  }
}

void SegmentTrackCombiner::matchTracksToSegment(SegmentInformation* segmentInformation,
                                                BaseSegmentTrackFilter& segmentTrackChooser)
{
  for (TrackInformation* trackInformation : m_trackLookUp) {
    double filterResult = segmentTrackChooser(std::make_pair(segmentInformation->getSegment(), trackInformation->getTrackCand()));
    if (isNotACell(filterResult)) {
      B2DEBUG(110, "Found not matchable in " << segmentInformation->getSegment()->getISuperLayer());
    } else {
      B2DEBUG(110, "Found matchable in " << segmentInformation->getSegment()->getISuperLayer());
      trackInformation->addMatch(segmentInformation, filterResult);
      segmentInformation->addMatch(trackInformation, filterResult);
    }
  }
}


void SegmentTrackCombiner::createTrainsOfMatchedSegments(std::list<TrainOfSegments>& trainsOfSegments,
                                                         const TrackInformation* trackInformation,
                                                         BaseSegmentTrainFilter& segmentTrainFilter)
{
  const std::vector<std::pair<SegmentInformation*, double>>& matchedSegments = trackInformation->getMatches();
  if (matchedSegments.size() == 1) {
    trainsOfSegments.push_back({ matchedSegments[0].first });
  } else if (matchedSegments.size() > 1 and matchedSegments.size() <= 5) {
    makeAllCombinations(trainsOfSegments, trackInformation, segmentTrainFilter);
  } else if (matchedSegments.size() > 5) {
    B2WARNING("Number of matched segments exceeds 5 with: " << matchedSegments.size());
  }
}
