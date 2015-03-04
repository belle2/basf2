/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFinderOutputCombiner/FittingMatrix.h>

//datastore types
#include <framework/datastore/StoreArray.h>

#include <framework/gearbox/Const.h>

#include <genfit/TrackCand.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <Eigen/Dense>
#include <genfit/WireTrackCandHit.h>
#include <cdc/dataobjects/CDCHit.h>


using namespace Belle2;
using namespace TrackFindingCDC;


void FittingMatrix::calculateMatrices(const std::vector<CDCRecoSegment2D>& recoSegments, const StoreArray<genfit::TrackCand>& resultTrackCands, const StoreArray<CDCHit>& cdcHits)
{
  // the functions to calculate the tree parameters probability for a good fit, z-momentum, start-z-distance to IP
  const CDCSZFitter& zFitter = CDCSZFitter::getFitter();
  const CDCRiemannFitter& circleFitter = CDCRiemannFitter::getFitter();

  m_fittingMatrix = Eigen::MatrixXf(recoSegments.size(), resultTrackCands.getEntries());
  m_zMatrix = Eigen::MatrixXf(recoSegments.size(), resultTrackCands.getEntries());
  m_zDistMatrix = Eigen::MatrixXf(recoSegments.size(), resultTrackCands.getEntries());
  m_segmentIsUsed.resize(recoSegments.size(), false);

  TrackCounter counterTracks = 0;
  for (const genfit::TrackCand & trackCand : resultTrackCands) {
    SegmentCounter counterSegment = 0;
    for (const CDCRecoSegment2D & segment : recoSegments) {

      m_fittingMatrix(counterSegment, counterTracks) = 0;
      m_zMatrix(counterSegment, counterTracks) = 0;
      m_zDistMatrix(counterSegment, counterTracks) = 0;

      CDCTrajectory3D trajectory(trackCand.getPosSeed(), trackCand.getMomSeed(), trackCand.getChargeSeed());

      // Calculate minimum and maximum angle:
      double minimumAngle = 4;
      double maximumAngle = 0;

      for (int cdcHitID : trackCand.getHitIDs(Const::CDC)) {
        CDCHit* cdcHit = cdcHits[cdcHitID];
        CDCWireHit cdcWireHit(cdcHit);
        double currentAngle = cdcWireHit.getPerpS(trajectory.getTrajectory2D());
        if (currentAngle > 0) {
          if (currentAngle < minimumAngle) {
            minimumAngle = currentAngle;
          }
          if (currentAngle > maximumAngle) {
            maximumAngle = currentAngle;
          }
        }
      }

      // For the reconstructed segment there are only two possible cases:
      // (1) the segment is full axial. We fit the hits of the segment together with the axial hits of the track.
      // (2) the segment is full stereo. We pull the hits of the segment to the track trajectory and fit the z direction
      if (segment.getStereoType() == AXIAL) {
        CDCObservations2D observationsCircle;

        // Add the hits from the segment to the circle fit
        for (const CDCRecoHit2D & recoHit2D : segment) {
          double currentAngle = recoHit2D.getPerpS(trajectory.getTrajectory2D());
          if (currentAngle < 1.5 * maximumAngle and currentAngle > 0.5 * minimumAngle)
            observationsCircle.append(recoHit2D);
        }

        // Add the hits from the TrackCand to the circle fit
        for (int cdcHitID : trackCand.getHitIDs(Const::CDC)) {
          CDCHit* cdcHit = cdcHits[cdcHitID];
          CDCWireHit cdcWireHit(cdcHit);
          if (cdcWireHit.getStereoType() == AXIAL) {
            observationsCircle.append(cdcWireHit);
          }
        }

        // Fit the circle trajectory
        CDCTrajectory2D trajectory2D;
        circleFitter.update(trajectory2D, observationsCircle);
        m_fittingMatrix(counterSegment, counterTracks) = TMath::Prob(trajectory2D.getChi2(), trajectory2D.getNDF());
        // we do not set the z-Matrix or the zDistMatrix here

      } else if (segment.getStereoType() == STEREO_U or segment.getStereoType() == STEREO_V) {
        CDCObservations2D observationsSZ;

        // Add the hits from the segment to the sz fit
        for (const CDCRecoHit2D & recoHit2D : segment) {
          const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory.getTrajectory2D());
          double currentAngle = recoHit3D.getPerpS(trajectory.getTrajectory2D());
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

void FittingMatrix::fillHitsInto(const CDCRecoSegment2D& recoSegment, genfit::TrackCand* bestTrackCand)
{
  // the sorting parameter is just used to reinforce the order in the range.
  int sortingParameter = -1;
  for (const auto & genHit : recoSegment) {
    ++sortingParameter;
    const CDCRLWireHit& rlWireHit = genHit->getRLWireHit();
    const CDCWireHit& wireHit = rlWireHit.getWireHit();
    const CDCWire& wire = rlWireHit.getWire();
    // the hit id correspondes to the index in the TClonesArray
    // this is stored in the wirehit the recohit is based on
    unsigned int storeIHit = wireHit.getStoreIHit();
    // the plain id serves to mark competition between two or more hits
    // use the wire id here which is unique for all the hits in the track
    // but it may also serve the fact that a track can only be responsable for a single hit on each wire
    // double hits should correspond to multiple tracks
    unsigned int planeId = wire.getEWire();
    // Right left ambiguity resolution
    RightLeftInfo rlInfo = rlWireHit.getRLInfo();
    // Note:  rlInfo < 0 means LEFT,   rlInfo > 0 means RIGHT,  which is the same as in Genfit
    signed char genfitLeftRight = rlInfo;
    //do not delete! the genfit::TrackCand has ownership
    genfit::WireTrackCandHit* cdcTrackCandHit = new genfit::WireTrackCandHit(
      Const::CDC, storeIHit, planeId, sortingParameter, genfitLeftRight);
    bestTrackCand->addHit(cdcTrackCandHit);
  }
}


void FittingMatrix::addSegmentToResultTrack(FittingMatrix::SegmentCounter counterSegment, FittingMatrix::TrackCounter counterTrack, const std::vector<TrackFindingCDC::CDCRecoSegment2D>& recoSegments, const StoreArray<genfit::TrackCand>& resultTrackCands)
{
  const TrackFindingCDC::CDCRecoSegment2D& recoSegment = recoSegments[counterSegment];
  genfit::TrackCand* trackCandidate = resultTrackCands[counterTrack];
  FittingMatrix::fillHitsInto(recoSegment, trackCandidate);
  resetSegment(counterSegment);
  m_segmentIsUsed[counterSegment] = true;
}

void FittingMatrix::resetSegment(SegmentCounter counterSegment)
{
  for (TrackCounter counterTracks = 0; counterTracks < m_fittingMatrix.cols(); counterTracks++) {
    resetEntry(counterSegment, counterTracks);
  }
}
