/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/trackFinderOutputCombining/FittingMatrix.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>
#include <tracking/trackFindingCDC/trackFinderOutputCombining/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <Eigen/Dense>


using namespace Belle2;
using namespace TrackFindingCDC;


void FittingMatrix::calculateMatrices(const std::vector<CDCRecoSegment2D>& recoSegments,
                                      const std::vector<CDCTrack>& resultTrackCands)
{
  // the functions to calculate the tree parameters probability for a good fit, z-momentum, start-z-distance to IP
  const CDCSZFitter& zFitter = CDCSZFitter::getFitter();
  const CDCRiemannFitter& circleFitter = CDCRiemannFitter::getFitter();

  m_fittingMatrix = Eigen::MatrixXf(recoSegments.size(), resultTrackCands.size());
  m_zMatrix = Eigen::MatrixXf(recoSegments.size(), resultTrackCands.size());
  m_zDistMatrix = Eigen::MatrixXf(recoSegments.size(), resultTrackCands.size());
  m_segmentIsUsed.clear();
  m_segmentIsUsed.resize(recoSegments.size(), false);

  TrackCounter counterTracks = 0;
  for (const CDCTrack& trackCand : resultTrackCands) {
    SegmentCounter counterSegment = 0;
    for (const CDCRecoSegment2D& segment : recoSegments) {

      m_fittingMatrix(counterSegment, counterTracks) = 0;
      m_zMatrix(counterSegment, counterTracks) = 0;
      m_zDistMatrix(counterSegment, counterTracks) = 0;

      if (segment.getAutomatonCell().hasTakenFlag()) {
        continue;
      }

      const CDCTrajectory3D& trajectory = trackCand.getStartTrajectory3D();

      // Calculate minimum and maximum angle:
      // TODO: Might not be needed, as we can assume the hits are sorted!
      double minimumAngle = trackCand.front().getPerpS();
      double maximumAngle = trackCand.back().getPerpS();

      // For the reconstructed segment there are only two possible cases:
      // (1) the segment is full axial. We fit the hits of the segment together with the axial hits of the track.
      // (2) the segment is full stereo. We pull the hits of the segment to the track trajectory and fit the z direction
      if (segment.getStereoType() == AXIAL) {
        CDCObservations2D observationsCircle;

        // Add the hits from the segment to the circle fit
        {
          const CDCTrajectory2D trajectory2D = trajectory.getTrajectory2D();
          for (const CDCRecoHit2D& recoHit2D : segment) {

            double currentAngle = trajectory2D.calcPerpS(recoHit2D.getRefPos2D());
            if (currentAngle < 1.5 * maximumAngle and currentAngle > 0.5 * minimumAngle)
              observationsCircle.append(recoHit2D);
          }
        }

        if (observationsCircle.size() > 0) {
          // Add the hits from the TrackCand to the circle fit
          for (const CDCRecoHit3D& recoHit : trackCand) {
            if (recoHit.getStereoType() == AXIAL) {
              observationsCircle.append(recoHit.getWireHit().getRefPos2D());
            }
          }

          // Fit the circle trajectory
          CDCTrajectory2D trajectory2D;
          circleFitter.update(trajectory2D, observationsCircle);
          m_fittingMatrix(counterSegment, counterTracks) = trajectory2D.getPValue();
          // we do not set the z-Matrix or the zDistMatrix here
        }
      } else if (segment.getStereoType() == STEREO_U or segment.getStereoType() == STEREO_V) {
        CDCObservations2D observationsSZ;

        // Add the hits from the segment to the sz fit
        const CDCTrajectory2D& trajectory2D = trajectory.getTrajectory2D();
        for (const CDCRecoHit2D& recoHit2D : segment) {
          const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory2D);
          double currentAngle = trajectory2D.calcPerpS(recoHit3D.getRecoPos2D());
          if (recoHit3D.isInCDC() and currentAngle < 1.5 * maximumAngle and currentAngle > 0.5 * minimumAngle) {
            observationsSZ.append(recoHit3D.getPerpS(), recoHit3D.getRecoPos3D().z());
          }
        }

        if (observationsSZ.size() > 3) {
          // Fit the sz trajectory
          CDCTrajectorySZ trajectorySZ;
          zFitter.update(trajectorySZ, observationsSZ);
          m_zMatrix(counterSegment, counterTracks) = TMath::ATan(trajectorySZ.getSZSlope());
          m_zDistMatrix(counterSegment, counterTracks) = trajectorySZ.getStartZ();
          m_fittingMatrix(counterSegment, counterTracks) = TMath::Prob(trajectorySZ.getChi2(), trajectorySZ.getNDF());
        }
      }
      counterSegment++;
    }
    counterTracks++;
  }
}


void FittingMatrix::addSegmentToResultTrack(FittingMatrix::SegmentCounter counterSegment, FittingMatrix::TrackCounter counterTrack,
                                            const std::vector<CDCRecoSegment2D>& recoSegments, std::vector<CDCTrack>& resultTrackCands)
{
  B2DEBUG(100, "Adding Segment #" << counterSegment << " with Track #" << counterTrack);
  const TrackFindingCDC::CDCRecoSegment2D& recoSegment = recoSegments[counterSegment];
  CDCTrack& trackCandidate = resultTrackCands[counterTrack];
  SegmentTrackCombiner::addSegmentToTrack(recoSegment, trackCandidate);
  resetSegment(counterSegment);
  m_segmentIsUsed[counterSegment] = true;
}


FittingMatrix::SegmentStatus FittingMatrix::calculateSegmentStatus(const CDCRecoSegment2D& segment, const CDCTrack& resultTrackCand)
{

  std::list<double> perpSList;
  std::list<double> perpSListOfSegment;

  const CDCTrajectory3D&   trajectory = resultTrackCand.getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory.getTrajectory2D();
  double radius = trajectory2D.getGlobalCircle().absRadius();

  const CDCRiemannFitter& circleFitter = CDCRiemannFitter::getFitter();

  // calculate all circle segments lengths
  for (const CDCRecoHit3D& recoHit : resultTrackCand) {
    double perpS;
    if (recoHit.getStereoType() == AXIAL) {
      perpS = trajectory2D.calcPerpS(recoHit.getRecoPos2D());
    } else {
      if (not recoHit.isInCDC())
        return SegmentStatus::NAN_IN_CALCULATION;
      perpS = recoHit.getPerpS();
    }

    if (perpS < 0) {
      perpS = perpS + 2 * TMath::Pi() * radius;
    }

    perpSList.push_back(perpS);
  }

  // sort the hits according to their segment length
  perpSList.sort();


  double endOfTrack = perpSList.back();
  double beginningOfTrack = perpSList.front();

  CDCObservations2D observationsCircle;
  for (const CDCRecoHit2D& recoHit : segment) {
    double perpS;
    if (recoHit.getStereoType() == AXIAL) {
      perpS = trajectory2D.calcPerpS(recoHit.getRecoPos2D());
      observationsCircle.append(recoHit);
    } else {
      TrackFindingCDC::CDCRecoHit3D recoHit3D = TrackFindingCDC::CDCRecoHit3D::reconstruct(recoHit, trajectory2D);
      perpS = recoHit3D.getPerpS();
    }

    if (perpS < 0) {
      perpS = perpS + 2 * TMath::Pi() * radius;
    }

    perpSListOfSegment.push_back(perpS);
  }

  // sort the hits according to their segment length
  perpSListOfSegment.sort();

  // does segment fit good to track?
  double endOfSegment = perpSListOfSegment.back();
  double beginningOfSegment = perpSListOfSegment.front();

  if (std::isnan(endOfSegment) or std::isnan(beginningOfSegment)) {
    return SegmentStatus::NAN_IN_CALCULATION;
  } else if (endOfSegment > endOfTrack) {
    return SegmentStatus::ABOVE_TRACK;
  } else if (beginningOfSegment < beginningOfTrack) {
    return SegmentStatus::BENEATH_TRACK;
  } else {

    // the segment lays in the track. We try to fit.
    std::list<double>::size_type beginningIndex = perpSList.size();
    std::list<double>::size_type endIndex = perpSList.size();

    unsigned int iter = 0;

    for (double perpS : perpSList) {
      if (static_cast<unsigned int>(beginningIndex) == perpSList.size() and perpS > beginningOfSegment) {
        if (iter == 0)
          beginningIndex = iter;
        else
          beginningIndex = iter - 1;
      }
      if (perpS < endOfSegment)
        endIndex = iter;

      iter++;
    }

    if (segment.getStereoType() == AXIAL) {
      beginningIndex = std::max(static_cast<std::list<double>::size_type>(0), beginningIndex - 5);
      endIndex = std::min(perpSList.size(), endIndex + 5);

      for (unsigned int counter = beginningIndex; counter < endIndex; counter++) {
        const CDCRecoHit3D& recoHit = resultTrackCand[counter];
        if (recoHit.getStereoType() == AXIAL) {
          observationsCircle.append(recoHit.getWireHit().getRefPos2D());
        } else {
          observationsCircle.append(recoHit.getRecoPos2D());
        }
      }

      CDCTrajectory2D segmentTrajectory;
      circleFitter.update(segmentTrajectory, observationsCircle);

      if (TMath::Prob(segmentTrajectory.getChi2(), segmentTrajectory.getNDF()) > 0)
        return SegmentStatus::IN_TRACK;
      else
        return SegmentStatus::NAN_IN_CALCULATION;

    } else {
      if (endIndex - beginningIndex == 0)
        return SegmentStatus::IN_TRACK;
      else
        return SegmentStatus::MIX_WITH_TRACK;
    }

  }
}
