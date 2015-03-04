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

//datastore types
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/DataStore.h>

#include <framework/gearbox/Const.h>

#include <genfit/TrackCand.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <Eigen/Dense>
#include <cdc/dataobjects/CDCHit.h>


using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(NotAssignedHitsCombiner);

NotAssignedHitsCombinerModule::NotAssignedHitsCombinerModule() : m_fittingMatrix()
{
  setDescription("Combines the newly found track candidates from the local track finder with the ones found by the legendre track finder to get all tracks.");

  addParam("TracksFromLegendreFinder",
           m_param_tracksFromLegendreFinder,
           "TrackCandidates store array name from the legendre track finder with hits from CDCHits.");
  addParam("CDCHits",
           m_param_cdcHits,
           "Name of the store array containing the hits from the legendre track finder.",
           std::string("CDCHits"));
  addParam("BadTrackCands",
           m_param_badTrackCands,
           "Name of the Store Array for the bad segments for testing.");
  addParam("RecoSegments",
           m_param_recoSegments,
           "Name of the Store Array for the segments from the local track finder.");
  addParam("ResultTrackCands",
           m_param_resultTrackCands,
           "TrackCandidates collection name from the combined results of the two recognition algorithm. The hits come from the parameter CDCHits.");
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
           "",
           0.95 * TMath::PiOver2());
  addParam("MaximumDistanceZ",
           m_fittingMatrix.getParamMaximumDistanceZ(),
           "",
           500.0);
  addParam("MinimalThetaDifference",
           m_fittingMatrix.getParamMinimalThetaDifference(),
           "",
           0.3);
  addParam("MinimalZDifference",
           m_fittingMatrix.getParamMinimalZDifference(),
           "",
           10.0);
}

void NotAssignedHitsCombinerModule::initialize()
{
  StoreArray<genfit::TrackCand> legendreTrackCands(m_param_tracksFromLegendreFinder);

  legendreTrackCands.isRequired();

  StoreArray<CDCHit> cdcHits(m_param_cdcHits);

  StoreArray<genfit::TrackCand> resultTrackCands(m_param_resultTrackCands);
  resultTrackCands.registerInDataStore();

  StoreArray<genfit::TrackCand> badTrackCands(m_param_badTrackCands);
  badTrackCands.isRequired();

  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> recoSegments(m_param_recoSegments);
  recoSegments.isRequired();
}

void NotAssignedHitsCombinerModule::findEasyCandidates(
  std::vector<CDCRecoSegment2D>& recoSegments,
  const StoreArray<genfit::TrackCand>& resultTrackCands,
  const StoreArray<genfit::TrackCand>& legendreTrackCands,
  const StoreArray<CDCHit>& cdcHits)
{
  // Find all "easy" possibilities:
  for (FittingMatrix::SegmentCounter counterOuter = 0;
       counterOuter < recoSegments.size(); counterOuter++) {
    unsigned int numberOfPossibleFits = 0; // How many tracks can be fitted to this segment in total
    unsigned int bestFitIndex = 0; // Which track has the highest chi2? For one possible fit partner this is the only possible index
    double highestChi2 = 0; // Which is the highest chi2?
    for (FittingMatrix::TrackCounter counterInner = 0;
         counterInner < resultTrackCands.getEntries(); counterInner++) {
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
    if (recoSegments[counterOuter].getStereoType() == AXIAL) {
      if (numberOfPossibleFits > 0
          && highestChi2 > m_fittingMatrix.getParamMinimalChi2()) {
        m_fittingMatrix.addSegmentToResultTrack(counterOuter, bestFitIndex,
                                                recoSegments, resultTrackCands);
      }
    } else if (numberOfPossibleFits == 1) {
      // Add segments with more or less the same parameters that fit to the track
      for (FittingMatrix::SegmentCounter counterSegments = 0;
           counterSegments < recoSegments.size(); counterSegments++) {
        if (m_fittingMatrix.segmentHasTheSameParameters(counterSegments,
                                                        counterOuter, bestFitIndex)) {
          m_fittingMatrix.addSegmentToResultTrack(counterSegments, bestFitIndex,
                                                  recoSegments, resultTrackCands);
        }
      }
    } else if (numberOfPossibleFits > 0) {
      // Now we come to the more tricky segments: we have more than one possibility
      // we look for the cases where we have more than one possible fit, but only one fits to the track:
      for (FittingMatrix::TrackCounter counterInner = 0;
           counterInner < resultTrackCands.getEntries(); counterInner++) {
        if (m_fittingMatrix.getChi2(counterOuter, counterInner)
            > m_fittingMatrix.getParamMinimalChi2()) {
          double theta = calculateThetaOfTrackCandidate(
                           legendreTrackCands[counterInner], cdcHits);
          if (std::abs(m_fittingMatrix.getZ(counterOuter, counterInner) - theta)
              < m_fittingMatrix.getParamMinimalThetaDifference()) {
            m_fittingMatrix.addSegmentToResultTrack(counterOuter, counterInner,
                                                    recoSegments, resultTrackCands);
          }
        }
      }
    }
  }
}

void NotAssignedHitsCombinerModule::event()
{
  B2DEBUG(100, "########## NotAssignedHitsCombinerModule ############")

  StoreArray<genfit::TrackCand> resultTrackCands(m_param_resultTrackCands);
  StoreArray<genfit::TrackCand> badTrackCands(m_param_badTrackCands);
  StoreArray<genfit::TrackCand> legendreTrackCands(m_param_tracksFromLegendreFinder);
  StoreArray<CDCHit> cdcHits(m_param_cdcHits);

  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments(m_param_recoSegments);
  std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;

  resultTrackCands.create();

  B2DEBUG(100, "Length of CDCSegments: " << recoSegments.size())


  // Add the legendre tracks to the output. They are assumed to be good at this stage.
  for (const genfit::TrackCand & legendreTrackCand : legendreTrackCands) {
    resultTrackCands.appendNew(legendreTrackCand);
  }

  // Go through all the reco segments
  // There are three different cases for a segment:
  // (1) It fits good to an already found legendre track candidate. We merge them.
  // (2) It does not fit good to a track candidate. Then:
  // (2a) It can be a new track
  // (2b) It can be background
  // As in most of the cases these segments are stereo segments, case (1) is the most common

  m_fittingMatrix.calculateMatrices(recoSegments, resultTrackCands, cdcHits);
  m_fittingMatrix.print();

  // Find all "easy" possibilities:
  findEasyCandidates(recoSegments, resultTrackCands, legendreTrackCands, cdcHits);


  // For the segments ending up here there are basically two possibilities:
  // (1) they are background -> actually these segments should be deleted before with the SegmentQualityCheckModule. We will not assume this case here!
  // (2) they form a new track - a track the legendre track finder has not found at all.
  // (3) the matching routine has not worked properly and they belong to an already found track
  // The question is not only if some of the segments are background - the question is also whether we should merge them in one ore more tracks

  unsigned int numberOfLeftSegments = 0;
  genfit::TrackCand* badTrackCand = badTrackCands.appendNew();
  for (FittingMatrix::SegmentCounter counterOuter = 0; counterOuter < recoSegments.size(); counterOuter++) {
    if (not m_fittingMatrix.isSegmentUsed(counterOuter)) {
      numberOfLeftSegments++;
      const CDCRecoSegment2D& recoSegment = recoSegments[counterOuter];
      m_fittingMatrix.fillHitsInto(recoSegment, badTrackCand);
    }
  }

  B2DEBUG(100, numberOfLeftSegments << " segments left.")

  m_fittingMatrix.print();
}

double NotAssignedHitsCombinerModule::calculateThetaOfTrackCandidate(genfit::TrackCand* trackCand, const StoreArray<CDCHit>& cdcHits)
{
  const CDCSZFitter& zFitter = CDCSZFitter::getFitter();

  CDCTrajectory3D trajectory(trackCand->getPosSeed(), trackCand->getMomSeed(), trackCand->getChargeSeed());
  CDCObservations2D observationsSZ;

  // Add the hits from the segment to the sz fit
  for (int cdcHitID : trackCand->getHitIDs(Const::CDC)) {
    CDCHit* cdcHit = cdcHits[cdcHitID];
    if (cdcHit->getISuperLayer() % 2 != 0) {
      // we do not know the right-left information
      CDCWireHit cdcWireHit(cdcHit);
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
    return 100;
  }
}
