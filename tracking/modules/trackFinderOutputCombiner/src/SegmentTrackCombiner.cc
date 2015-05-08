#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <genfit/TrackCand.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombiner);

SegmentTrackCombinerModule::SegmentTrackCombinerModule() : TrackFinderCDCFromSegmentsModule()
{
  addParam("LegendreTrackCandsStoreArrayName", m_param_legendreTrackCandsStoreArrayName,
           "The StoreArray Name for the LegendreTrackCands we use for combining.",
           std::string("LegendreTrackCands"));
}

void SegmentTrackCombinerModule::initialize()
{
  TrackFinderCDCFromSegmentsModule::initialize();

  StoreArray<genfit::TrackCand> legendreTrackCands(m_param_legendreTrackCandsStoreArrayName);
  legendreTrackCands.isRequired();
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
    matchingTracks.erase(
    std::remove_if(matchingTracks.begin(), matchingTracks.end(), [&segmentInformation](TrackInformation * possiblyMatch) -> bool {
      double perpSFront = possiblyMatch->getTrajectory().calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
      if (perpSFront > possiblyMatch->getMaxPerpS() or perpSFront < possiblyMatch->getMinPerpS())
      {
        B2DEBUG(120, "Segment is above or below track.")
        return true;
      } else {
        return false;
      }
    }), matchingTracks.end());

    if (matchingTracks.size() > 1) {
      B2DEBUG(100, matchingTracks.size() << " are too many possible partners! Aborting")
      matchingTracks.clear();
      return;
    } else if (matchingTracks.size() == 0) {
      B2DEBUG(100, "None of the matches were in the track. Aborting. TODO: There may be matches above the track!")
      return;
    } else {
      B2DEBUG(100, "Combining segment with track: " << matchingTracks[0]->getTrajectory())

      for (const CDCRecoHit2D& recoHit : segmentInformation->getSegment()->items()) {
        matchingTracks[0]->getTrackCand()->addHit(Const::CDC, recoHit.getWireHit().getStoreIHit(), recoHit.getRLInfo());
        matchingTracks[0]->getPerpSList().push_back(matchingTracks[0]->getTrajectory().calcPerpS(recoHit.getRecoPos2D()));
      }

      matchingTracks[0]->calcPerpS();
      matchingTracks.clear();
    }
  }
}

void SegmentTrackCombinerModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  //CDCRiemannFitter circleFitter = Belle2.TrackFindingCDC.CDCRiemannFitter.getFitter();

  StoreArray<genfit::TrackCand> legendreTrackCands(m_param_legendreTrackCandsStoreArrayName);
  StoreArray<CDCHit> cdcHits;


  // Calculate a lookup SuperLayerID -> Segments
  typedef std::vector<SegmentInformation> SegmentsList;

  std::vector<SegmentsList> segmentLookUp;
  segmentLookUp.resize(wireTopology.N_SUPERLAYERS);

  for (CDCRecoSegment2D& segment : segments) {
    ILayerType superlayerID = segment.getISuperLayer();
    segmentLookUp[superlayerID].push_back(SegmentInformation(&segment));
  }

  // Calculate a lookup Track -> TrackInformation
  std::vector<TrackInformation> trackLookUp;

  for (genfit::TrackCand& trackCand : legendreTrackCands) {
    TrackInformation trackInformation(&trackCand);

    CDCTrajectory3D trajectory(trackCand.getPosSeed(), trackCand.getMomSeed(), trackCand.getChargeSeed());
    genfit::TrackCandHit* hit = trackCand.getHit(0);
    const CDCWireHit* wireHit = wireHitTopology.getWireHit(cdcHits[hit->getHitId()]);
    trajectory.setLocalOrigin(Vector3D(wireHit->getRefPos2D(), 0));
    trackInformation.setTrajectory(trajectory.getTrajectory2D());

    std::vector<double>& perpSList = trackInformation.getPerpSList();
    perpSList.reserve(trackCand.getNHits());
    for (unsigned int hitID = 0; hitID < trackCand.getNHits(); hitID++) {
      genfit::TrackCandHit* hit = trackCand.getHit(hitID);
      CDCHit* cdcHit = cdcHits[hit->getHitId()];
      const CDCWireHit* wireHit = wireHitTopology.getWireHit(cdcHit);
      if (wireHit->getStereoType() == AXIAL) {
        perpSList.push_back(trackInformation.getTrajectory().calcPerpS(wireHit->getRefPos2D()));
      } else {
        // TODO: Actually we need the rl-information here!
        CDCRLWireHit rlWireHit(wireHit, hit->getPlaneId());
        const Vector3D& reconstructed3D = rlWireHit.reconstruct3D(trackInformation.getTrajectory());
        perpSList.push_back(trackInformation.getTrajectory().calcPerpS(reconstructed3D.xy()));
      }
    }
    trackInformation.calcPerpS();
    trackLookUp.push_back(trackInformation);
  }

  unsigned int superLayerCounter = 0;
  for (SegmentsList& segmentsList : segmentLookUp) {
    superLayerCounter++;
    B2DEBUG(100, "Starting superlayer " << superLayerCounter - 1);


    // Search for all matching tracks for a given segment
    for (SegmentInformation& segmentInformation : segmentsList) {
      for (TrackInformation& trackInformation : trackLookUp) {
        if (segmentMatchesToTrack(segmentInformation, trackInformation)) {
          B2DEBUG(110, "Found matchable in " << superLayerCounter - 1)
          trackInformation.getMatchingSegments().push_back(&segmentInformation);
          segmentInformation.getMatchingTracks().push_back(&trackInformation);
        } else {
          B2DEBUG(110, "Found not matchable in " << superLayerCounter - 1)
        }
      }
    }

    // Go through all tracks.
    for (TrackInformation& trackInformation : trackLookUp) {
      ListOfSegmentPointer& matchedSegments = trackInformation.getMatchingSegments();

      B2DEBUG(100, "Matched Segments: " << matchedSegments.size());

      for (SegmentInformation* matchedSegment : matchedSegments) {
        B2DEBUG(110, matchedSegment->getMatchingTracks().size());
      }
      // Try to find the longest trains of segments
      std::list<ListOfSegmentPointer> trainsOfSegments;

      if (matchedSegments.size() == 0) {
        continue;
      } else if (matchedSegments.size() == 1) {
        trainsOfSegments.push_back(matchedSegments);
      } else if (matchedSegments.size() > 1) {
        makeAllCombinations(trainsOfSegments, matchedSegments, trackInformation);
      }

      if (trainsOfSegments.size() == 1) {
        // If there is only one matching segment/train, mark this as matched
        // TODO: Better: Add this to a list!
        combineSegmentTrainAndTrack(trainsOfSegments.front());
      } else if (trainsOfSegments.size() > 1) {
        // If there are more possibilities use the one which fills in the empty space of the track best
        B2DEBUG(100, "Number of possible trains: " << trainsOfSegments.size());
      }
    }

    // Reset the matching segments lists
    for (TrackInformation& trackInformation : trackLookUp) {
      trackInformation.clearMatchingSegments();
    }
  }

  // Delete all used segments
  /*segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) -> bool {
    return segment.size() == 0 or segment.getAutomatonCell().hasTakenFlag();
  }), segments.end());*/

  // Copy all result tracks to the output array
  for (TrackInformation& trackInformation : trackLookUp) {
    tracks.emplace_back();
    CDCTrack& track = tracks.back();
    CDCTrajectory3D trajectory(trackInformation.getTrajectory(), CDCTrajectorySZ::basicAssumption());
    track.setStartTrajectory3D(trajectory);

    genfit::TrackCand* trackCand = trackInformation.getTrackCand();

    for (unsigned int hitID = 0; hitID < trackCand->getNHits(); hitID++) {
      genfit::TrackCandHit* hit = trackCand->getHit(hitID);
      CDCHit* cdcHit = cdcHits[hit->getHitId()];
      int planeID = hit->getPlaneId();
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(cdcHit, planeID);
      const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackInformation.getTrajectory());
      track.push_back(recoHit3D);
    }
  }
}
