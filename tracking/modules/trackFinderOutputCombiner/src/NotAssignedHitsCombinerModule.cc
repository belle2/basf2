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

#include <tracking/trackFinderOutputCombiner/TrackFinderOutputCombiner.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>
#include <tracking/trackFindingCDC/rootification/StoreWrapper.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <genfit/WireTrackCandHit.h>
#include <Eigen/Dense>
#include <cdc/dataobjects/CDCHit.h>


using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(NotAssignedHitsCombiner);

namespace {

  template <typename Type>
  TrackFinderOutputCombiner::BestFitInformation calculateBestFitPartner(const StoreArray<Type>& listWithItems, std::function<double(const Type&)> calculator)
  {
    TrackFinderOutputCombiner::BestFitInformation fitInformation(-1, -1);
    // Find best fit partner
    unsigned int counter = 0;
    for (const Type & item : listWithItems) {
      double prob = calculator(item);
      if (prob > fitInformation.second) {
        fitInformation.second = prob;
        fitInformation.first = counter;
      }
      counter++;
    }

    return fitInformation;
  }

  template <typename Type1, typename Type2>
  std::vector<TrackFinderOutputCombiner::BestFitInformation> calculateBestFitPartnerList(const StoreArray<Type1>& listWithType1, const StoreArray<Type2>& listWithType2, std::function<double(const Type1&, const Type2&)> calculator)
  {
    unsigned int counter = 0;
    std::vector<TrackFinderOutputCombiner::BestFitInformation> fitInformationList;
    for (const Type1 & outerItem : listWithType1) {
      std::function<double(const Type2&)> calculatorFunction = [&](const Type2 & innerItem) -> double {
        return calculator(outerItem, innerItem);
      };

      TrackFinderOutputCombiner::BestFitInformation fitInformation = calculateBestFitPartner(listWithType2, calculatorFunction);

      if (fitInformation.first != -1) {
        B2DEBUG(100, "Best candidate for " << counter << " is " << fitInformation.first << " with " << fitInformation.second);
      }
      counter++;
      fitInformationList.push_back(fitInformation);
    }

    return fitInformationList;
  }

  template <typename Type1, typename Type2>
  Eigen::MatrixXf calculateFittingMatrix(const std::vector<Type1>& listWithType1, const StoreArray<Type2>& listWithType2, std::function<double(const Type1&, const Type2&)> calculator)
  {
    Eigen::MatrixXf fittingMatrix(listWithType1.size(), listWithType2.getEntries());

    unsigned int counterOuter = 0;
    for (const Type1 & outerItem : listWithType1) {
      unsigned int counterInner = 0;
      for (const Type2 & innerItem : listWithType2) {

        fittingMatrix(counterOuter, counterInner) = calculator(outerItem, innerItem);

        counterInner++;
      }
      counterOuter++;
    }

    return fittingMatrix;
  }
}

NotAssignedHitsCombinerModule::NotAssignedHitsCombinerModule()
{
  setDescription("Combines the newly found track candidates from the local track finder with the ones found by the legendre track finder to get all tracks.");

  addParam("TracksFromLegendreFinder",
           m_param_tracksFromLegendreFinder,
           "TrackCandidates store array name from the legendre track finder with hits from CDCHits.",
           std::string("PreLegendreTrackCands"));
  addParam("NotAssignedTracksFromLocalFinder",
           m_param_notAssignedTracksFromLocalFinder,
           "TrackCandidates store array name from the local track finder with hits from NotAssignedCDCHits. These tracks will be converted into tracks with hits from CDCHits",
           std::string("NotAssignedLocalTrackCands"));
  addParam("CDCHits",
           m_param_cdcHits,
           "Name of the store array containing the hits from the legendre track finder.",
           std::string("CDCHits"));

  addParam("ResultTrackCands",
           m_param_resultTrackCands,
           "TrackCandidates collection name from the combined results of the two recognition algorithm. The hits come from the parameter CDCHits.",
           std::string("ResultTrackCands"));
  addParam("MinimalChi2",
           m_param_minimal_chi2,
           "Minimal chi2 for a merge.",
           0.80);
  addParam("MinimalChi2Stereo",
           m_param_minimal_chi2_stereo,
           "Minimal chi2 for a merge of stereo segments to a track candidate. This should be really small because fitting stereo segments is a difficult task.",
           0.00001);

  addParam("MaximumTheta",
           m_param_maximum_momentum_z,
           "",
           0.95 * TMath::PiOver2());
  addParam("MaximumDistanceZ",
           m_param_maximum_distance_z,
           "",
           500.0);
  addParam("MinimalThetaDifference",
           m_param_minimal_theta_difference,
           "",
           0.3);
  addParam("MinimalZDifference",
           m_param_minimal_z_difference,
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

  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> recoSegments("CDCRecoSegment2DVector");
  recoSegments.isRequired();
}

void NotAssignedHitsCombinerModule::fillHitsInto(const CDCRecoSegment2D& recoSegment, genfit::TrackCand* bestTrackCand)
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

void NotAssignedHitsCombinerModule::calculateFittingMatrices(const StoreArray<CDCHit>& cdcHits, const StoreArray<genfit::TrackCand>& resultTrackCands, const std::vector<CDCRecoSegment2D>& recoSegments)
{
  // the functions to calculate the tree parameters probability for a good fit, z-momentum, start-z-distance to IP
  const CDCSZFitter& zFitter = CDCSZFitter::getFitter();

  std::function<double(const CDCRecoSegment2D&, const genfit::TrackCand&)> function = [&](const CDCRecoSegment2D & segment, const genfit::TrackCand & trackCand) -> double {
    return calculateGoodFitIndex(trackCand, segment, cdcHits);
  };

  std::function<double(const CDCRecoSegment2D&, const genfit::TrackCand&)> zFunction = [&](const CDCRecoSegment2D & segment, const genfit::TrackCand & trackCand) -> double {
    if (segment.getStereoType() == STEREO_U or segment.getStereoType() == STEREO_V) {
      CDCTrajectory3D trajectory(trackCand.getPosSeed(), trackCand.getMomSeed(), trackCand.getChargeSeed());
      CDCObservations2D observationsSZ;

      // Add the hits from the segment to the sz fit
      for (const CDCRecoHit2D & recoHit2D : segment) {
        const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory.getTrajectory2D());
        if (recoHit3D.isInCDC() and recoHit3D.getPerpS() / trajectory.getTrajectory2D().getGlobalCircle().absRadius()) {
          observationsSZ.append(recoHit3D.getPerpS(), recoHit3D.getRecoPos3D().z());
        }
      }

      if (observationsSZ.size() > 3) {
        // Fit the sz trajectory
        CDCTrajectorySZ trajectorySZ;
        zFitter.update(trajectorySZ, observationsSZ);
        return TMath::ATan(trajectorySZ.getSZSlope());
      }
    }

    return 0;
  };

  std::function<double(const CDCRecoSegment2D&, const genfit::TrackCand&)> zDistFunction = [&](const CDCRecoSegment2D & segment, const genfit::TrackCand & trackCand) -> double {
    if (segment.getStereoType() == STEREO_U or segment.getStereoType() == STEREO_V) {
      CDCTrajectory3D trajectory(trackCand.getPosSeed(), trackCand.getMomSeed(), trackCand.getChargeSeed());
      CDCObservations2D observationsSZ;

      // Add the hits from the segment to the sz fit
      for (const CDCRecoHit2D & recoHit2D : segment) {
        const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory.getTrajectory2D());
        if (recoHit3D.isInCDC()) {
          observationsSZ.append(recoHit3D.getPerpS(), recoHit3D.getRecoPos3D().z());
        }
      }

      if (observationsSZ.size() > 3) {
        // Fit the sz trajectory
        CDCTrajectorySZ trajectorySZ;
        zFitter.update(trajectorySZ, observationsSZ);
        return trajectorySZ.getStartZ();
      }
    }

    return 0;
  };

  // Calculate the matrices
  m_fittingMatrix = calculateFittingMatrix(recoSegments, resultTrackCands, function);
  m_zMatrix = calculateFittingMatrix(recoSegments, resultTrackCands, zFunction);
  m_zDistMatrix = calculateFittingMatrix(recoSegments, resultTrackCands, zDistFunction);
}

void NotAssignedHitsCombinerModule::deleteWrongFits(const StoreArray<genfit::TrackCand>& resultTrackCands, std::vector<CDCRecoSegment2D>& recoSegments)
{
  // Delete all "wrong" fits:
  for (unsigned int counterOuter = 0; counterOuter < recoSegments.size();
       counterOuter++) {
    for (int counterInner = 0; counterInner < resultTrackCands.getEntries();
         counterInner++) {
      if (m_fittingMatrix(counterOuter, counterInner)
          < m_param_minimal_chi2_stereo
          || std::abs(m_zMatrix(counterOuter, counterInner))
          > m_param_maximum_momentum_z
          || std::abs(m_zDistMatrix(counterOuter, counterInner))
          > m_param_maximum_distance_z) {
        resetEntry(m_fittingMatrix, counterOuter, m_zMatrix, m_zDistMatrix,
                   counterInner);
      }
    }
  }
}

void NotAssignedHitsCombinerModule::event()
{
  B2DEBUG(100, "########## NotAssignedHitsCombinerModule ############")

  StoreArray<genfit::TrackCand> resultTrackCands(m_param_resultTrackCands);
  StoreArray<genfit::TrackCand> legendreTrackCands(m_param_tracksFromLegendreFinder);
  StoreArray<CDCHit> cdcHits(m_param_cdcHits);

  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments("CDCRecoSegment2DVector");
  std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;

  //matchNewlyFoundLocalTracksToLegendreTracks(resultTrackCands, localTrackCands, legendreTrackCands, cdcHits);


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


  calculateFittingMatrices(cdcHits, resultTrackCands, recoSegments);
  deleteWrongFits(resultTrackCands, recoSegments);

  B2DEBUG(100, "Fitting Matrix:\n" << m_fittingMatrix)
  B2DEBUG(100, "z Matrix:\n" << m_zMatrix)
  B2DEBUG(100, "z Dist Matrix:\n" << m_zDistMatrix)

  std::vector<CDCRecoSegment2D> notMatchedAxialSegments;

  // Find all "easy" possibilities:
  for (unsigned int counterOuter = 0; counterOuter < recoSegments.size(); counterOuter++) {
    unsigned int numberOfPossibleFits = 0; // How many tracks can be fittest to this segment in total
    unsigned int bestFitIndex = 0;         // Which track has the highest chi2? For one possible fit partner this is the only possible index
    double highestChi2 = 0;                // Which is the highest chi2?

    for (int counterInner = 0; counterInner < resultTrackCands.getEntries(); counterInner++) {
      if (m_fittingMatrix(counterOuter, counterInner) > 0) {
        numberOfPossibleFits++;
        if (m_fittingMatrix(counterOuter, counterInner) > highestChi2) {
          bestFitIndex = counterInner;
          highestChi2 = m_fittingMatrix(counterOuter, counterInner);
        }
      }
    }

    // for axial hits we add the segment to the best fit partner
    // for stereo hits we add it to the track candidate, if we have only one possible fit partner. Then, we add all other segments sharing more or less the same parameters
    if (recoSegments[counterOuter].getStereoType() == AXIAL) {
      const CDCRecoSegment2D& recoSegment = recoSegments[counterOuter];
      if (numberOfPossibleFits > 0 and highestChi2 > m_param_minimal_chi2) {
        genfit::TrackCand* trackCandidate = resultTrackCands[bestFitIndex];
        fillHitsInto(recoSegment, trackCandidate);
        resetEntry(m_fittingMatrix, counterOuter, m_zMatrix, m_zDistMatrix, bestFitIndex);
      } else {
        notMatchedAxialSegments.push_back(recoSegment);
      }
    } else if (numberOfPossibleFits == 1) {
      double zReference = m_zMatrix(counterOuter, bestFitIndex);
      double zDistReference = m_zDistMatrix(counterOuter, bestFitIndex);

      // Add segments with more or less the same parameters that fit to the track
      for (unsigned int counterSegments = 0; counterSegments < recoSegments.size(); counterSegments++) {
        if (m_fittingMatrix(counterSegments, bestFitIndex) > 0 and
            std::abs(m_zMatrix(counterSegments, bestFitIndex) - zReference) < m_param_minimal_theta_difference / 2 and
            std::abs(m_zDistMatrix(counterSegments, bestFitIndex) - zDistReference) < m_param_minimal_z_difference) {
          const CDCRecoSegment2D& recoSegment = recoSegments[counterSegments];
          genfit::TrackCand* trackCandidate = resultTrackCands[bestFitIndex];
          fillHitsInto(recoSegment, trackCandidate);
          resetEntry(m_fittingMatrix, counterSegments, m_zMatrix, m_zDistMatrix, bestFitIndex);
        }
      }
    } else if (numberOfPossibleFits > 0)  {
      // Now we come to the more tricky segments: we have more than one possibility
      // we look for the cases where we have more than one possible fit, but only one fits to the track:
      for (int counterInner = 0; counterInner < resultTrackCands.getEntries(); counterInner++) {
        if (m_fittingMatrix(counterOuter, counterInner) > m_param_minimal_chi2) {
          double theta = calculateThetaOfTrackCandidate(legendreTrackCands[counterInner], cdcHits);
          if (std::abs(m_zMatrix(counterOuter, counterInner) - theta)
              < m_param_minimal_theta_difference) {
            genfit::TrackCand* trackCandidate = resultTrackCands[counterInner];
            const CDCRecoSegment2D& recoSegment = recoSegments[counterOuter];
            fillHitsInto(recoSegment, trackCandidate);
            resetEntry(m_fittingMatrix, counterOuter, m_zMatrix, m_zDistMatrix, counterInner);
          }
        }
      }
    }
  }


  // For the segments ending up here there are basically two possibilities:
  // (1) they are background
  // (2) they form a new track - a track the legendre track finder has not found at all.
  // The question is not only if some of the segments are background - the question is also whether we should merge them in one ore more tracks

  unsigned int numberOfLeftSegments = 0;
  for (unsigned int counterOuter = 0; counterOuter < recoSegments.size(); counterOuter++) {
    for (int counterInner = 0; counterInner < resultTrackCands.getEntries(); counterInner++) {
      if (m_fittingMatrix(counterOuter, counterInner) > 0) {
        numberOfLeftSegments++;
        break;
      }
    }
  }

  B2INFO(numberOfLeftSegments << " segments left.")

  B2DEBUG(100, "Fitting Matrix:\n" << m_fittingMatrix)
  B2DEBUG(100, "z Matrix:\n" << m_zMatrix)
  B2DEBUG(100, "z Dist Matrix:\n" << m_zDistMatrix)
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

double NotAssignedHitsCombinerModule::calculateGoodFitIndex(const genfit::TrackCand& trackCandidate, const CDCRecoSegment2D segment, const StoreArray<CDCHit>& cdcHits)
{
  // Construct a trajectory with the initial parameters of the track candidate for reference
  CDCTrajectory3D trajectory(trackCandidate.getPosSeed(), trackCandidate.getMomSeed(), trackCandidate.getChargeSeed());

  const CDCRiemannFitter& circleFitter = CDCRiemannFitter::getFitter();
  const CDCSZFitter& zFitter = CDCSZFitter::getFitter();

  // Calculate minimum and maximum angle:
  double minimumAngle = 4;
  double maximumAngle = 0;

  for (int cdcHitID : trackCandidate.getHitIDs(Const::CDC)) {
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
      observationsCircle.append(recoHit2D.getRecoPos2D());
    }

    // Add all axial hits from the track candidate to the circle fit. As we do not have a reconstructed position, we use the reference position of the wire.
    for (unsigned int cdcHitID : trackCandidate.getHitIDs(Const::CDC))  {
      CDCHit* cdcHit = cdcHits[cdcHitID];
      CDCWireHit cdcWireHit(cdcHit);
      if (cdcWireHit.getStereoType() == AXIAL) {
        observationsCircle.append(cdcWireHit.getRefPos2D());
      }
    }

    // Fit the circle trajectory
    CDCTrajectory2D trajectory2D;
    circleFitter.update(trajectory2D, observationsCircle);
    return TMath::Prob(trajectory2D.getChi2(), trajectory2D.getNDF());

  } else if (segment.getStereoType() == STEREO_U or segment.getStereoType() == STEREO_V) {

    CDCObservations2D observationsSZ;

    // Add the hits from the segment to the sz fit
    for (const CDCRecoHit2D & recoHit2D : segment) {
      const CDCRecoHit3D& recoHit3D = CDCRecoHit3D::reconstruct(recoHit2D, trajectory.getTrajectory2D());
      double currentAngle = recoHit3D.getPerpS(trajectory.getTrajectory2D());
      // this hit can only be added to the track if it is (after reconstruction) still in the CDC and has a reasonable perpS.
      if (recoHit3D.isInCDC() and currentAngle >= 0.8 * minimumAngle and currentAngle <= 1.2 * maximumAngle) {
        observationsSZ.append(recoHit3D.getPerpS(), recoHit3D.getRecoPos3D().z());
      }
    }

    if (observationsSZ.size() > 3) {
      // Fit the sz trajectory
      CDCTrajectorySZ trajectorySZ;
      zFitter.update(trajectorySZ, observationsSZ);

      return TMath::Prob(trajectorySZ.getChi2(), trajectorySZ.getNDF());
    } else {
      return 0;
    }

  } else {
    // We can not handle other cases.
    return 0;
  }
}
