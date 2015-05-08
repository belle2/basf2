#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <genfit/TrackCand.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombiner);

SegmentTrackCombinerModule::SegmentTrackCombinerModule() : TrackFinderCDCFromSegmentsModule(), m_segmentLookUp(), m_trackLookUp()
{
  addParam("LegendreTrackCandsStoreArrayName", m_param_legendreTrackCandsStoreArrayName,
           "The StoreArray Name for the LegendreTrackCands we use for combining.",
           std::string("LegendreTrackCands"));
}

void SegmentTrackCombinerModule::initialize()
{
  TrackFinderCDCFromSegmentsModule::initialize();
  m_legendreTrackCands.isRequired(m_param_legendreTrackCandsStoreArrayName);
  m_cdcHits.isRequired();
}

void SegmentTrackCombinerModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  // Calculate a lookup SuperLayerID -> Segments
  calculateSegmentLookUp(segments);

  // Calculate a lookup Track -> TrackInformation
  calculateTrackLookup();

  unsigned int superLayerCounter = 0;
  for (SegmentsList& segmentsList : m_segmentLookUp) {
    superLayerCounter++;
    B2DEBUG(100, "Starting superlayer " << superLayerCounter - 1);

    // Search for all matching tracks for a given segment
    for (SegmentInformation& segmentInformation : segmentsList) {
      matchTracksToSegments(superLayerCounter, segmentInformation);
    }

    // Go through all tracks.
    for (TrackInformation& trackInformation : m_trackLookUp) {

      // Try to find the longest trains of segments
      std::list<ListOfSegmentPointer> trainsOfSegments;
      createTrainsOfSegments(trainsOfSegments, trackInformation);

      if (trainsOfSegments.size() == 1) {
        // If there is only one matching segment/train, mark this as matched
        // TODO: Better: Add this to a list!
        combineSegmentTrainAndTrack(trainsOfSegments.front());
      } else if (trainsOfSegments.size() > 1) {
        // If there are more possibilities use the one which fills in the empty space of the track best
        B2WARNING("Number of possible trains still left: " << trainsOfSegments.size());
      }
    }

    // Reset the matching segments lists
    for (TrackInformation& trackInformation : m_trackLookUp) {
      trackInformation.clearMatchingSegments();
    }

    unsigned int notUsedSegments = std::count_if(segmentsList.begin(),
    segmentsList.end(), [](const SegmentInformation & segmentInformation) -> bool {
      if (segmentInformation.isUsedInTrack())
      {
        segmentInformation.getSegment()->getAutomatonCell().setTakenFlag();
        return false;
      } else {
        segmentInformation.getSegment()->getAutomatonCell().unsetTakenFlag();
        return true;
      }
    });

    if (notUsedSegments > 0) {
      B2WARNING("Still " << notUsedSegments << " not used in this superlayer.")
    }

  }

  // Delete all used segments
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) -> bool {
    return segment.size() == 0 or segment.getAutomatonCell().hasTakenFlag();
  }), segments.end());

  B2WARNING("After all there are " << segments.size() << " Segments left in this event.")

  // Copy all result tracks to the output array
  createTracksForOutput(tracks);
}

bool SegmentTrackCombinerModule::segmentMatchesToTrack(const SegmentInformation& segmentInformation,
                                                       const TrackInformation& trackInformation)
{

  const CDCRecoHit2D& front = segmentInformation.getSegment()->front();
  const CDCRecoHit2D& back = segmentInformation.getSegment()->back();

  // Calculate distance
  const CDCTrajectory2D& trajectory = trackInformation.getTrajectory();

  if (segmentInformation.getSegment()->getStereoType() != AXIAL) {
    if (fabs(trajectory.getDist2D(front.getRecoPos2D())) > 10)  {
      B2DEBUG(120, "Hits too far away.")
      return false;
    }
  } else {
    if (fabs(trajectory.getDist2D(front.getRecoPos2D())) > 5)  {
      B2DEBUG(120, "Hits too far away.")
      return false;
    }
  }

  Vector3D frontRecoPos3D = front.reconstruct3D(trajectory);
  Vector3D backRecoPos3D = back.reconstruct3D(trajectory);

  if (segmentInformation.getSegment()->getStereoType() != AXIAL) {
    double forwardZ = front.getWire().getSkewLine().forwardZ();
    double backwardZ = front.getWire().getSkewLine().backwardZ();

    if (frontRecoPos3D.z() > forwardZ or frontRecoPos3D.z() < backwardZ) {
      B2DEBUG(120, "Segment out of CDC after reconstruction.")
      return false;
    }
  }

  // Calculate perpS
  double perpS_min = trajectory.calcPerpS(frontRecoPos3D.xy());
  double perpS_max = trajectory.calcPerpS(backRecoPos3D.xy());

  if (perpS_min > perpS_max) {
    std::swap(perpS_min, perpS_max);
  }

  unsigned int hitsInSameRegion = 0;
  for (double perpS : trackInformation.getPerpSList()) {
    if (perpS < (1 + m_param_percentageForPerpSMeasurements) * perpS_max
        and perpS > (1 - m_param_percentageForPerpSMeasurements) * perpS_min) {
      hitsInSameRegion++;
    }
  }

  if (hitsInSameRegion > 3) {
    B2DEBUG(110, "Too many hits in the same region: " << hitsInSameRegion)
    return false;
  } else {
    B2DEBUG(110, "Hits in the region " << perpS_min << " - " << perpS_max << ": " << hitsInSameRegion << " with in segment: " <<
            segmentInformation.getSegment()->size())
  }

  return true;
}

bool SegmentTrackCombinerModule::doesFitTogether(const ListOfSegmentPointer& list, const TrackInformation& trackInformation)
{
  double lastPerpS;
  bool alreadySet = false;
  for (SegmentInformation* segmentInformation : list) {
    double perpSFront = trackInformation.getTrajectory().calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
    if (alreadySet and perpSFront < (1 - m_param_percentageForPerpSMeasurements) * lastPerpS) {
      return false;
    }
    alreadySet = true;
    lastPerpS = trackInformation.getTrajectory().calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());
  }
  return true;
}

void SegmentTrackCombinerModule::makeAllCombinations(std::list<ListOfSegmentPointer>& trainsOfSegments,
                                                     const ListOfSegmentPointer& matchedSegments, const TrackInformation& trackInformation)
{
  trainsOfSegments.clear();
  trainsOfSegments.emplace_back();

  for (SegmentInformation* segment : matchedSegments) {
    std::list<ListOfSegmentPointer> innerSet;
    for (ListOfSegmentPointer x : trainsOfSegments) {
      x.push_back(segment);
      std::sort(x.begin(), x.end(), [&trackInformation](SegmentInformation * first, SegmentInformation * second) {
        return trackInformation.getTrajectory().calcPerpS(first->getSegment()->front().getRecoPos2D()) >
               trackInformation.getTrajectory().calcPerpS(second->getSegment()->front().getRecoPos2D());
      });
      if (doesFitTogether(x, trackInformation))
        innerSet.push_back(x);
    }
    trainsOfSegments.insert(trainsOfSegments.end(), innerSet.begin(), innerSet.end());
  }
}

void SegmentTrackCombinerModule::addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTrack)
{
  segmentInformation->setUsedInTrack();
  for (const CDCRecoHit2D& recoHit : segmentInformation->getSegment()->items()) {
    matchingTrack->getTrackCand()->addHit(Const::CDC,
                                          recoHit.getWireHit().getStoreIHit(), recoHit.getRLInfo());
    matchingTrack->getPerpSList().push_back(
      matchingTrack->getTrajectory().calcPerpS(recoHit.getRecoPos2D()));
  }
  matchingTrack->calcPerpS();
}

double SegmentTrackCombinerModule::testFitSegmentToTrack(SegmentInformation* segmentInformation,
                                                         TrackInformation* trackInformation)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  CDCObservations2D observations;

  double perpSFront = trackInformation->getTrajectory().calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
  double perpSBack = trackInformation->getTrajectory().calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());

  genfit::TrackCand* trackCand = trackInformation->getTrackCand();

  bool isAxialSegment = segmentInformation->getSegment()->getStereoType() != AXIAL;

  for (unsigned int hitID = 0; hitID < trackCand->getNHits(); hitID++) {
    genfit::TrackCandHit* hit = trackCand->getHit(hitID);
    CDCHit* cdcHit = m_cdcHits[hit->getHitId()];
    const CDCWireHit* wireHit = wireHitTopology.getWireHit(cdcHit);
    if (isAxialSegment) {
      observations.append(wireHit->getRefPos2D());
    } else {
      // TODO: Actually we need the rl-information here!
      CDCRLWireHit rlWireHit(wireHit, hit->getPlaneId());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trackInformation->getTrajectory());
      double s = recoHit3D.getPerpS();
      double z = recoHit3D.getRecoZ();
      observations.append(s, z);
    }
  }

  for (const CDCRecoHit2D& recoHit : segmentInformation->getSegment()->items()) {
    if (isAxialSegment) {
      observations.append(recoHit.getRecoPos2D());
    } else {
      // TODO: Actually we need the rl-information here!
      CDCRLWireHit rlWireHit(recoHit.getWireHit(), recoHit.getRLInfo());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trackInformation->getTrajectory());
      double s = recoHit3D.getPerpS();
      double z = recoHit3D.getRecoZ();
      observations.append(s, z);
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

void SegmentTrackCombinerModule::combineSegmentTrainAndTrack(const ListOfSegmentPointer& trainOfSegments)
{
  B2DEBUG(100, "Trying to combine...")
  for (SegmentInformation* segmentInformation : trainOfSegments) {

    std::vector<TrackInformation*>& matchingTracks = segmentInformation->getMatchingTracks();

    if (matchingTracks.size() == 0) {
      B2DEBUG(100, "Match was deleted.")
      return;
    }

    // Try to fiddle out which is the best one!
    std::vector<TrackInformation*> matchesAboveTrack;
    matchesAboveTrack.reserve(matchingTracks.size() / 2);

    matchingTracks.erase(
      std::remove_if(matchingTracks.begin(), matchingTracks.end(), [&segmentInformation,
    &matchesAboveTrack](TrackInformation * possiblyMatch) -> bool {
      double perpSFront = possiblyMatch->getTrajectory().calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
      double perpSBack = possiblyMatch->getTrajectory().calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());
      if (perpSFront > possiblyMatch->getMaxPerpS() or perpSBack < possiblyMatch->getMinPerpS())
      {
        B2DEBUG(120, "Segment is above or below track.")
        matchesAboveTrack.push_back(possiblyMatch);
        return true;
      } else {
        return false;
      }
    }), matchingTracks.end());

    if (matchingTracks.size() > 1) {
      B2DEBUG(100, matchingTracks.size() << " are too many possible partners! We choose the best one:")
      double bestFitProb = 0;
      TrackInformation* bestMatch = nullptr;

      for (TrackInformation* trackInformation : matchingTracks) {
        double fitProbability = testFitSegmentToTrack(segmentInformation, trackInformation);

        if (fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track after fit test: " << bestMatch->getTrajectory() << " with: " << bestFitProb)
        addSegmentToTrack(segmentInformation, bestMatch);
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
        B2DEBUG(100, "Combining segment with track above/below after fit test: " << bestMatch->getTrajectory() << " with: " << bestFitProb)
        addSegmentToTrack(segmentInformation, bestMatch);
      }
    } else {
      B2DEBUG(100, "Combining segment with track: " << matchingTracks[0]->getTrajectory())
      addSegmentToTrack(segmentInformation, matchingTracks[0]);
    }

    matchingTracks.clear();
  }
}

void SegmentTrackCombinerModule::calculateSegmentLookUp(std::vector<CDCRecoSegment2D>& segments)
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  // Calculate a lookup SuperLayerID -> Segments
  m_segmentLookUp.clear();
  m_segmentLookUp.resize(wireTopology.N_SUPERLAYERS);
  for (CDCRecoSegment2D& segment : segments) {
    ILayerType superlayerID = segment.getISuperLayer();
    m_segmentLookUp[superlayerID].push_back(SegmentInformation(&segment));
  }
}

void SegmentTrackCombinerModule::calculateTrackLookup()
{
  // Actually we will not need this once the legendre track finder creates cdcTracks also
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  m_trackLookUp.clear();
  m_trackLookUp.reserve(m_legendreTrackCands.getEntries());

  // Calculate a lookup Track -> TrackInformation
  for (genfit::TrackCand& trackCand : m_legendreTrackCands) {
    TrackInformation trackInformation(&trackCand);
    CDCTrajectory3D trajectory(trackCand.getPosSeed(), trackCand.getMomSeed(),
                               trackCand.getChargeSeed());
    genfit::TrackCandHit* hit = trackCand.getHit(0);
    const CDCWireHit* wireHit = wireHitTopology.getWireHit(
                                  m_cdcHits[hit->getHitId()]);
    trajectory.setLocalOrigin(Vector3D(wireHit->getRefPos2D(), 0));
    trackInformation.setTrajectory(trajectory.getTrajectory2D());
    std::vector<double>& perpSList = trackInformation.getPerpSList();
    perpSList.reserve(trackCand.getNHits());
    for (unsigned int hitID = 0; hitID < trackCand.getNHits(); hitID++) {
      genfit::TrackCandHit* hit = trackCand.getHit(hitID);
      CDCHit* cdcHit = m_cdcHits[hit->getHitId()];
      const CDCWireHit* wireHit = wireHitTopology.getWireHit(cdcHit);
      if (wireHit->getStereoType() == AXIAL) {
        perpSList.push_back(
          trackInformation.getTrajectory().calcPerpS(wireHit->getRefPos2D()));
      } else {
        // TODO: Actually we need the rl-information here!
        CDCRLWireHit rlWireHit(wireHit, hit->getPlaneId());
        const Vector3D& reconstructed3D = rlWireHit.reconstruct3D(
                                            trackInformation.getTrajectory());
        perpSList.push_back(
          trackInformation.getTrajectory().calcPerpS(reconstructed3D.xy()));
      }
    }
    trackInformation.calcPerpS();
    m_trackLookUp.push_back(trackInformation);
  }
}

void SegmentTrackCombinerModule::createTracksForOutput(std::vector<CDCTrack>& tracks)
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  // Copy all result tracks to the output array
  for (TrackInformation& trackInformation : m_trackLookUp) {
    tracks.emplace_back();
    CDCTrack& track = tracks.back();
    CDCTrajectory3D trajectory(trackInformation.getTrajectory(),
                               CDCTrajectorySZ::basicAssumption());
    track.setStartTrajectory3D(trajectory);
    genfit::TrackCand* trackCand = trackInformation.getTrackCand();
    for (unsigned int hitID = 0; hitID < trackCand->getNHits(); hitID++) {
      genfit::TrackCandHit* hit = trackCand->getHit(hitID);
      CDCHit* cdcHit = m_cdcHits[hit->getHitId()];
      int planeID = hit->getPlaneId();
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(cdcHit,
                                      planeID);
      const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit,
                                                                trackInformation.getTrajectory());
      track.push_back(recoHit3D);
    }
  }
}

void SegmentTrackCombinerModule::matchTracksToSegments(
  unsigned int superLayerCounter, SegmentInformation& segmentInformation)
{
  for (TrackInformation& trackInformation : m_trackLookUp) {
    if (segmentMatchesToTrack(segmentInformation, trackInformation)) {
      B2DEBUG(110, "Found matchable in " << superLayerCounter - 1)
      trackInformation.getMatchingSegments().push_back(&segmentInformation);
      segmentInformation.getMatchingTracks().push_back(&trackInformation);
    } else {
      B2DEBUG(110, "Found not matchable in " << superLayerCounter - 1)
    }
  }
}

void SegmentTrackCombinerModule::createTrainsOfSegments(std::list<ListOfSegmentPointer>& trainsOfSegments,
                                                        TrackInformation& trackInformation)
{
  ListOfSegmentPointer& matchedSegments = trackInformation.getMatchingSegments();

  B2DEBUG(100, "Matched Segments: " << matchedSegments.size());

  for (SegmentInformation* matchedSegment : matchedSegments) {
    B2DEBUG(110, matchedSegment->getMatchingTracks().size());
  }

  if (matchedSegments.size() == 0) {
  } else if (matchedSegments.size() == 1) {
    trainsOfSegments.push_back(matchedSegments);
  } else if (matchedSegments.size() > 1) {
    makeAllCombinations(trainsOfSegments, matchedSegments, trackInformation);
  }
}
