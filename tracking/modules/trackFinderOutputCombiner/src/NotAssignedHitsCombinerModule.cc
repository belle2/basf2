/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/NotAssignedHitsCombinerModule.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>


using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(NotAssignedHitsCombiner);

NotAssignedHitsCombinerModule::NotAssignedHitsCombinerModule() : TrackFinderCDCFromSegmentsModule(), m_fittingMatrix(),
  m_param_useSecondStage(false)
{
  setDescription("Combines the newly found track candidates from the local track finder with the ones found by the legendre track finder to get all tracks.");

  addParam("MinimalChi2",
           m_fittingMatrix.getParamMinimalChi2(),
           "Minimal chi2 for a merge.",
           0.80);
  addParam("MinimalChi2Stereo",
           m_fittingMatrix.getParamMinimalChi2Stereo(),
           "Minimal chi2 for a merge of stereo segments to a track candidate. This should be really small because fitting stereo segments is a difficult task.",
           0.00001);
  addParam("MaximumTheta",
           m_fittingMatrix.getParamMaximumTheta(),
           "Maximum Theta a track is allowed to have to be called matchable.",
           0.95 * TMath::PiOver2());
  addParam("MaximumDistanceZ",
           m_fittingMatrix.getParamMaximumDistanceZ(),
           "Maximum Distance in z two fits should have to be called matchable.",
           500.0);
  addParam("MinimalThetaDifference",
           m_fittingMatrix.getParamMinimalThetaDifference(),
           "",
           0.3);
  addParam("MinimalZDifference",
           m_fittingMatrix.getParamMinimalZDifference(),
           "",
           10.0);
  addParam("UseSecondStage",
           m_param_useSecondStage,
           "Use the second combining stage",
           m_param_useSecondStage);
}


void NotAssignedHitsCombinerModule::generate(std::vector<CDCRecoSegment2D>& segments, std::vector<CDCTrack>& tracks)
{
  B2DEBUG(100, "########## NotAssignedHitsCombinerModule ############")

  // Go through all the reco segments
  // There are three different cases for a segment:
  // (1) It fits good to an already found legendre track candidate. We merge them.
  // (2) It does not fit good to a track candidate. Then:
  // (2a) It can be a new track
  // (2b) It can be background
  // As in most of the cases these segments are stereo segments, case (1) is the most common

  // Find all "easy" possibilities:
  findEasyCandidates(segments, tracks);
  m_fittingMatrix.print();


  // For the segments ending up here there are basically two possibilities:
  // (1) they are background -> actually these segments should be deleted before with the SegmentQualityCheckModule. We will not assume this case here!
  // (2) they form a new track - a track the legendre track finder has not found at all.
  // (3) the matching routine has not worked properly and they belong to an already found track
  // The question is not only if some of the segments are background - the question is also whether we should merge them in one ore more tracks

  // Delete all segments, that are used
  std::vector<CDCRecoSegment2D> notUsedRecoSegments;
  for (FittingMatrix::SegmentCounter segmentCounter = 0; segmentCounter < segments.size(); segmentCounter++) {
    if (not m_fittingMatrix.isSegmentUsed(segmentCounter)) {
      notUsedRecoSegments.push_back(std::move(segments[segmentCounter]));
    }
  }

  //findHarderCandidates(notUsedRecoSegments, tracks);
  m_fittingMatrix.print();

  // Delete all segments, that are used
  for (FittingMatrix::SegmentCounter segmentCounter = 0; segmentCounter < segments.size(); segmentCounter++) {
    if (m_fittingMatrix.isSegmentUsed(segmentCounter)) {
      segments[segmentCounter].getAutomatonCell().setTakenFlag();
    }
  }
  segments.erase(std::remove_if(segments.begin(), segments.end(), [](const CDCRecoSegment2D & segment) -> bool { return segment.getAutomatonCell().hasTakenFlag(); }),
                 segments.end());
}


void NotAssignedHitsCombinerModule::findEasyCandidates(std::vector<CDCRecoSegment2D>& segments,
                                                       std::vector<CDCTrack>& tracks)
{
  m_fittingMatrix.calculateMatrices(segments, tracks);
  m_fittingMatrix.print();

  // Make a copy of the tracks
  std::vector<CDCTrack> copy;
  std::copy(tracks.begin(), tracks.end(), std::back_inserter(copy));

  // Find all "easy" possibilities:
  for (FittingMatrix::SegmentCounter counterOuter = 0;
       counterOuter < segments.size(); counterOuter++) {
    unsigned int numberOfPossibleFits = 0; // How many tracks can be fitted to this segment in total
    unsigned int bestFitIndex = 0; // Which track has the highest chi2? For one possible fit partner this is the only possible index
    double highestChi2 = 0; // Which is the highest chi2?
    for (FittingMatrix::TrackCounter counterInner = 0;
         counterInner < tracks.size(); counterInner++) {
      if (m_fittingMatrix.isGoodEntry(counterOuter, counterInner)) {
        numberOfPossibleFits++;
        if (m_fittingMatrix.getChi2(counterOuter, counterInner) > highestChi2) {
          bestFitIndex = counterInner;
          highestChi2 = m_fittingMatrix.getChi2(counterOuter, counterInner);
        }
      }
    }
    // for axial hits we add the segment to the best fit partner
    // for stereo hits we add it to the track candidate, if we have only one possible fit partner. Then, we add all other segments sharing more or less the same parameters
    if (segments[counterOuter].getStereoType() == AXIAL) {
      if (numberOfPossibleFits > 0
          && highestChi2 > m_fittingMatrix.getParamMinimalChi2()) {
        m_fittingMatrix.addSegmentToResultTrack(counterOuter, bestFitIndex,
                                                segments, tracks);
      }
    } else if (numberOfPossibleFits == 1) {
      // Add segments with more or less the same parameters that fit to the track
      for (FittingMatrix::SegmentCounter counterSegments = 0;
           counterSegments < segments.size(); counterSegments++) {
        if (m_fittingMatrix.segmentHasTheSameParameters(counterSegments,
                                                        counterOuter, bestFitIndex)) {
          m_fittingMatrix.addSegmentToResultTrack(counterSegments, bestFitIndex,
                                                  segments, tracks);
        }
      }
    } else if (numberOfPossibleFits > 0) {
      // Now we come to the more tricky segments: we have more than one possibility
      // we look for the cases where we have more than one possible fit, but only one fits to the track:
      for (FittingMatrix::TrackCounter counterInner = 0;
           counterInner < tracks.size(); counterInner++) {
        if (m_fittingMatrix.getChi2(counterOuter, counterInner)
            > m_fittingMatrix.getParamMinimalChi2()) {
          double theta = calculateThetaOfTrackCandidate(copy[counterInner]);
          if (std::abs(m_fittingMatrix.getZ(counterOuter, counterInner) - theta)
              < m_fittingMatrix.getParamMinimalThetaDifference()) {
            m_fittingMatrix.addSegmentToResultTrack(counterOuter, counterInner,
                                                    segments, tracks);
          }
        }
      }
    }
  }
}

void NotAssignedHitsCombinerModule::findHarderCandidates(std::vector<CDCRecoSegment2D>& notUsedRecoSegments,
                                                         std::vector<CDCTrack>& tracks)
{

  m_fittingMatrix.calculateMatrices(notUsedRecoSegments, tracks);
  m_fittingMatrix.print();

  // Check if the segments would fit into a whole in a track pattern of a result track candidate
  for (FittingMatrix::SegmentCounter segmentCounter = 0; segmentCounter < notUsedRecoSegments.size(); segmentCounter++) {
    const CDCRecoSegment2D& recoSegment = notUsedRecoSegments[segmentCounter];

    StereoType stereoType = recoSegment.getStereoType();

    std::vector<FittingMatrix::SegmentStatus> status;
    status.reserve(tracks.size());

    for (const CDCTrack& resultTrackCand : tracks) {
      const CDCTrajectory3D& trajectory = resultTrackCand.getStartTrajectory3D();
      const CDCTrajectory2D& trajectory2D = trajectory.getTrajectory2D();

      // if it is axial, we can check the mean distance to the track in the xy-plane
      if (stereoType == AXIAL) {
        double meanDistanceToTrack = 0;

        for (const CDCRecoHit2D& recoHit : recoSegment) {
          meanDistanceToTrack += recoHit.getSquaredDist2D(trajectory2D);
        }

        if (meanDistanceToTrack / recoSegment.size() > 100) {
          status.push_back(FittingMatrix::SegmentStatus::NAN_IN_CALCULATION);
          continue;
        }
      }

      FittingMatrix::SegmentStatus segmentStatus = FittingMatrix::calculateSegmentStatus(recoSegment, resultTrackCand);
      status.push_back(segmentStatus);
    }

    unsigned int numberOfInMatches = std::count_if(status.begin(), status.end(), [](FittingMatrix::SegmentStatus s) -> bool { return s == FittingMatrix::SegmentStatus::IN_TRACK or s == FittingMatrix::SegmentStatus::MIX_WITH_TRACK; });
    if (stereoType == AXIAL) {
      if (numberOfInMatches >= 1) {

        FittingMatrix::TrackCounter bestFitPartner = -1;
        double maximumChi2 = -1;

        for (FittingMatrix::TrackCounter trackCounter = 0; trackCounter < tracks.size(); trackCounter++) {
          if (status[trackCounter] == FittingMatrix::SegmentStatus::IN_TRACK
              or status[trackCounter] == FittingMatrix::SegmentStatus::MIX_WITH_TRACK) {
            if (m_fittingMatrix.getChi2(segmentCounter, trackCounter) > maximumChi2) {
              maximumChi2 = m_fittingMatrix.getChi2(segmentCounter, trackCounter);
              bestFitPartner = trackCounter;
            }
          }
        }

        if (numberOfInMatches == 1 or status[bestFitPartner] == FittingMatrix::SegmentStatus::IN_TRACK or maximumChi2 > 0) {
          B2DEBUG(100, "Adding " << segmentCounter << " to " << bestFitPartner << " with: " << maximumChi2);
          m_fittingMatrix.addSegmentToResultTrack(segmentCounter, bestFitPartner, notUsedRecoSegments, tracks);
        }
      }
    }

    B2INFO("Above: " << std::count_if(status.begin(), status.end(), [](FittingMatrix::SegmentStatus s) -> bool{ return s == FittingMatrix::SegmentStatus::ABOVE_TRACK; }))
    B2INFO("Below: " << std::count_if(status.begin(), status.end(), [](FittingMatrix::SegmentStatus s) -> bool{ return s == FittingMatrix::SegmentStatus::BENEATH_TRACK; }))
    B2INFO("In: " << std::count_if(status.begin(), status.end(), [](FittingMatrix::SegmentStatus s) -> bool{ return s == FittingMatrix::SegmentStatus::IN_TRACK; }))
    B2INFO("Mix: " << std::count_if(status.begin(), status.end(), [](FittingMatrix::SegmentStatus s) -> bool{ return s == FittingMatrix::SegmentStatus::MIX_WITH_TRACK; }))

    B2INFO("Axial: " << (recoSegment.getStereoType() == AXIAL))
  }
}

double NotAssignedHitsCombinerModule::calculateThetaOfTrackCandidate(const TrackFindingCDC::CDCTrack& trackCandidate)
{
  const CDCSZFitter& zFitter = CDCSZFitter::getFitter();

  const CDCTrajectory3D& trajectory = trackCandidate.getStartTrajectory3D();
  CDCObservations2D observationsSZ;

  // TODO: Is it the same if adding the already fitted values??

  // Add the hits from the segment to the sz fit
  for (CDCRecoHit3D recoHit : trackCandidate) {
    if (recoHit.getStereoType() != AXIAL) {
      // we do not know the right-left information
      CDCWireHit cdcWireHit(recoHit.getWireHit());
      CDCRLWireHit cdcRLWireHit(cdcWireHit);
      const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(cdcRLWireHit, trajectory.getTrajectory2D());
      if (recoHit3D.isInCDC()) {
        observationsSZ.append(recoHit3D.getPerpS(), recoHit3D.getRecoPos3D().z());
      }
    }
  }

  if (observationsSZ.size() > 3) {
    // Fit the sz trajectory
    CDCTrajectorySZ trajectorySZ;
    zFitter.update(trajectorySZ, observationsSZ);
    return TMath::ATan(trajectorySZ.getSZSlope());
  } else {
    // just a dummy value: this means the track is more or less axial only
    B2WARNING("Axial only")
    return 100;
  }
}
