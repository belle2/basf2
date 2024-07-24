/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/kinkFinding/fitter/KinkFitter.h>

#include <framework/logging/Logger.h>
#include <framework/geometry/VectorUtil.h>
#include <framework/geometry/BFieldManager.h>
#include <framework/utilities/IOIntercept.h>
#include <framework/geometry/B2Vector3.h>

#include <tracking/trackFitting/trackBuilder/factories/TrackBuilder.h>

#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/Track.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>

#include <genfit/TrackPoint.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/KalmanFitterRefTrack.h>

#include <analysis/VertexFitting/KFit/VertexFitKFit.h>
#include <analysis/utility/ROOTToCLHEP.h>
#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Geometry/Point3D.h>

using namespace Belle2;

KinkFitter::KinkFitter(const std::string& trackFitResultsName, const std::string& kinksName,
                       const std::string& recoTracksName, const std::string& copiedRecoTracksName)
  : m_recoTracksName(recoTracksName), m_kinkFitterMode(1)
{
  m_trackFitResults.isRequired(trackFitResultsName);
  m_kinks.registerInDataStore(kinksName, DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);

  /// m_recoTracks
  m_recoTracks.isRequired(m_recoTracksName);

  /// register m_copiedRecoTracks
  m_copiedRecoTracks.registerInDataStore(copiedRecoTracksName,
                                         DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_copiedRecoTracks);

  /// relation : m_recoTracks <--> m_copiedRecoTracks
  m_copiedRecoTracks.registerRelationTo(m_recoTracks);

  B2ASSERT("Material effects not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::MaterialEffects::getInstance()->isInitialized());
  B2ASSERT("Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::FieldManager::getInstance()->isInitialized());
}

void KinkFitter::setFitterMode(const unsigned char fitterMode)
{
  if (not(fitterMode <= 15)) {
    B2FATAL("Invalid fitter mode!");
  } else {
    m_kinkFitterMode = fitterMode;
    // filling bits of the fitter mode
    m_kinkFitterModeHitsReassignment = fitterMode & 0b0001;
    m_kinkFitterModeFlipAndRefit = fitterMode & 0b0010;
    m_kinkFitterModeCombineAndFit = fitterMode & 0b0100;
    m_kinkFitterModeSplitTrack = fitterMode & 0b1000;
  }
}

void KinkFitter::initializeCuts(const double vertexDistanceCut,
                                const double vertexChi2Cut,
                                const double precutDistance)
{
  m_vertexDistanceCut = vertexDistanceCut;
  m_vertexChi2Cut = vertexChi2Cut;
  m_precutDistance = precutDistance;
}

/// Extrapolate the states to the fitted vertex. If the vertex is inside one of the tracks, bits are set.
bool KinkFitter::extrapolateToVertex(genfit::MeasuredStateOnPlane& stMother, genfit::MeasuredStateOnPlane& stDaughter,
                                     const ROOT::Math::XYZVector& vertexPos, unsigned int& reassignHitStatus)
{
  reassignHitStatus = 0;
  try {
    // extrapolate the state to the vertexPos
    // the value will be positive (negative) if the direction of the extrapolation is (counter)momentum-wise
    double extralengthMother = stMother.extrapolateToPoint(XYZToTVector(vertexPos));
    double extralengthDaughter = stDaughter.extrapolateToPoint(XYZToTVector(vertexPos));
    if (extralengthMother > 0
        && extralengthDaughter > 0) reassignHitStatus |= 0x1; // both positive means daughter hits to be reassigned to mother
    if (extralengthMother < 0
        && extralengthDaughter < 0) reassignHitStatus |= 0x2; // both negative means mother hits to be reassigned to daughter
    B2DEBUG(29, "extralengthMother=" << extralengthMother << ", extralengthDaughter=" << extralengthDaughter);
  } catch (...) {
    // Ideally, this shouldn't happen
    B2DEBUG(29, "Could not extrapolate track to vertex.");
    return false;
  }
  return true;
}

/// Build TrackFitResult of Kink Track.
TrackFitResult* KinkFitter::buildTrackFitResult(RecoTrack* recoTrack,
                                                const genfit::MeasuredStateOnPlane& msop,
                                                const double Bz,
                                                const Const::ParticleType trackHypothesis)
{
  const uint64_t hitPatternCDCInitializer = TrackBuilder::getHitPatternCDCInitializer(*recoTrack);
  const uint32_t hitPatternVXDInitializer = TrackBuilder::getHitPatternVXDInitializer(*recoTrack);
  const genfit::FitStatus* trackFitStatus = recoTrack->getTrackFitStatus();

  TrackFitResult* kinkTrackFitResult
    = m_trackFitResults.appendNew(ROOT::Math::XYZVector(msop.getPos()), ROOT::Math::XYZVector(msop.getMom()),
                                  msop.get6DCov(), msop.getCharge(),
                                  trackHypothesis,
                                  trackFitStatus->getPVal(),
                                  Bz, hitPatternCDCInitializer, hitPatternVXDInitializer,
                                  trackFitStatus->getNdf());
  return kinkTrackFitResult;
}

/// Find hit position closest to the vertex.
int KinkFitter::findHitPositionForReassignment(const RecoTrack* recoTrack,
                                               ROOT::Math::XYZVector& vertexPos,
                                               int direction)
{

  if (direction != 1 && direction != -1) {
    B2WARNING("KinkFitter::findHitPositionForReassignment: the direction is not +-1, although should be. "
              "Set to +1 (-1) for direction > 0 (< 0).");
    if (direction > 0)
      direction = 1;
    else
      direction = -1;
  }

  // Helper variables to store the minimum
  double minimalDistance2 = std::numeric_limits<double>::max();
  int minimalIndex = 0;
  int riHit;

  // CDC Hits list to loop over
  auto cdcHits = recoTrack->getSortedCDCHitList();


  for (int cdcHitIndex = 0; cdcHitIndex < static_cast<int>(cdcHits.size()); ++cdcHitIndex) {
    if (direction > 0)
      riHit = cdcHitIndex;
    else
      riHit = static_cast<int>(cdcHits.size()) - 1 - cdcHitIndex;

    auto recoHitInfo = recoTrack->getRecoHitInformation(cdcHits[riHit]);
    if (!recoHitInfo->useInFit()) continue;
    try {
      const genfit::MeasuredStateOnPlane& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromRecoHit(
          recoHitInfo);
      const double currentDistance2 = (ROOT::Math::XYZVector(measuredStateOnPlane.getPos()) - vertexPos).Mag2();

      if (currentDistance2 < minimalDistance2) {
        minimalDistance2 = currentDistance2;
        minimalIndex = cdcHitIndex;
      }
      // if it cannot find minimum in 3 iterations, stop searching
      if (cdcHitIndex - minimalIndex > 3) break;
    } catch (const NoTrackFitResult& exception) {
      B2DEBUG(29, "Can not get mSoP because of: " << exception.what());
      continue;
    } catch (const genfit::Exception& exception) {
      B2DEBUG(29, "Can not get mSoP because of: " << exception.what());
      continue;
    }
  }
  if (minimalIndex == 0)
    return -1 * direction;
  else
    return -(minimalIndex * direction);

}

/// Copy RecoTrack to a separate StoreArray and reassign CDC hits according to delta
RecoTrack* KinkFitter::copyRecoTrackAndReassignCDCHits(RecoTrack* motherRecoTrack, RecoTrack* daughterRecoTrack,
                                                       const bool motherFlag, const int delta)
{

  // lists of CDC hits of mother and daughter tracks
  const auto motherCDCHit = motherRecoTrack->getSortedCDCHitList();
  const auto daughterCDCHit = daughterRecoTrack->getSortedCDCHitList();

  // initialization of helper variables
  const int deltaMother = delta > 0 ? -delta : 0;
  const int deltaDaughter = delta < 0 ? -delta : 0;
  int sortingParameterOffset = 0;

  // pointer to a track to be copied
  RecoTrack* recoTrackToCopy = nullptr;
  if (motherFlag) {
    recoTrackToCopy = motherRecoTrack;
  } else {
    recoTrackToCopy = daughterRecoTrack;
  }

  // copy recoTracks to a separate StoreArray using seed information
  RecoTrack* recoTrack = m_copiedRecoTracks.appendNew(ROOT::Math::XYZVector(recoTrackToCopy->getPositionSeed()),
                                                      ROOT::Math::XYZVector(recoTrackToCopy->getMomentumSeed()),
                                                      static_cast<short>(recoTrackToCopy->getChargeSeed()),
                                                      recoTrackToCopy->getStoreArrayNameOfPXDHits(),
                                                      recoTrackToCopy->getStoreArrayNameOfSVDHits(),
                                                      recoTrackToCopy->getStoreArrayNameOfCDCHits(),
                                                      recoTrackToCopy->getStoreArrayNameOfBKLMHits(),
                                                      recoTrackToCopy->getStoreArrayNameOfEKLMHits(),
                                                      recoTrackToCopy->getStoreArrayNameOfRecoHitInformation());
  recoTrack->setTimeSeed(recoTrackToCopy->getTimeSeed());
  recoTrack->setSeedCovariance(recoTrackToCopy->getSeedCovariance());

  if (motherFlag) {

    // copy PXD hits (we have checked in KinkFinderModule that there are no PXD hits on the other side of the track)
    for (const auto* pxdHit : recoTrackToCopy->getPXDHitList()) {
      auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(pxdHit);
      recoTrack->addPXDHit(pxdHit, recoHitInfo->getSortingParameter(),
                           recoHitInfo->getFoundByTrackFinder());
    }

    // copy SVD hits (we have checked in KinkFinderModule that there are no SVD hits on the other side of the track)
    for (const auto* svdHit : recoTrackToCopy->getSVDHitList()) {
      auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(svdHit);
      recoTrack->addSVDHit(svdHit, recoHitInfo->getSortingParameter(),
                           recoHitInfo->getFoundByTrackFinder());
    }

    // copy CDC hits with respect to reassignment
    for (size_t motherCDCHitIndex = 0; motherCDCHitIndex < motherCDCHit.size() + deltaMother; ++motherCDCHitIndex) {
      auto recoHitInfo = motherRecoTrack->getRecoHitInformation(motherCDCHit[motherCDCHitIndex]);
      recoTrack->addCDCHit(motherCDCHit[motherCDCHitIndex],
                           recoHitInfo->getSortingParameter(),
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }
    sortingParameterOffset = recoTrack->getNumberOfTotalHits();
    for (size_t daughterCDCHitIndex = 0; daughterCDCHitIndex < static_cast<unsigned int>(deltaDaughter); ++daughterCDCHitIndex) {
      auto recoHitInfo = daughterRecoTrack->getRecoHitInformation(daughterCDCHit[daughterCDCHitIndex]);
      recoTrack->addCDCHit(daughterCDCHit[daughterCDCHitIndex],
                           recoHitInfo->getSortingParameter() + sortingParameterOffset,
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }

    // we do not want to have KLM hits in mother track even if they exist

  } else {

    // In case of hit reassignment, we do not want to have VXD hits in the beginning of daughter track
    // even if they exist (absense of VXD hits at the end of the track was checked in KinkFinderModule)

    if (deltaMother)
      sortingParameterOffset = motherRecoTrack->getRecoHitInformation(motherCDCHit[motherCDCHit.size() +
                                                   deltaMother])->getSortingParameter();
    // copy CDC hits with respect to reassignment
    for (size_t motherCDCHitIndex = motherCDCHit.size() + deltaMother; motherCDCHitIndex < motherCDCHit.size(); ++motherCDCHitIndex) {
      auto recoHitInfo = motherRecoTrack->getRecoHitInformation(motherCDCHit[motherCDCHitIndex]);
      recoTrack->addCDCHit(motherCDCHit[motherCDCHitIndex],
                           recoHitInfo->getSortingParameter() - sortingParameterOffset,
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }
    sortingParameterOffset = -deltaMother - deltaDaughter;
    for (size_t daughterCDCHitIndex = deltaDaughter; daughterCDCHitIndex < daughterCDCHit.size(); ++daughterCDCHitIndex) {
      auto recoHitInfo = daughterRecoTrack->getRecoHitInformation(daughterCDCHit[daughterCDCHitIndex]);
      recoTrack->addCDCHit(daughterCDCHit[daughterCDCHitIndex],
                           recoHitInfo->getSortingParameter() + sortingParameterOffset,
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }

    // copy BKLM hits
    for (const auto* bklmHit : recoTrackToCopy->getBKLMHitList()) {
      auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(bklmHit);
      recoTrack->addBKLMHit(bklmHit, recoHitInfo->getSortingParameter() + sortingParameterOffset,
                            recoHitInfo->getFoundByTrackFinder());
    }

    // copy EKLM hits
    for (const auto* eklmHit : recoTrackToCopy->getEKLMHitList()) {
      auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(eklmHit);
      recoTrack->addEKLMHit(eklmHit, recoHitInfo->getSortingParameter() + sortingParameterOffset,
                            recoHitInfo->getFoundByTrackFinder());
    }
  }

  return recoTrack;

}

/// Try to fit new RecoTracks after hit reassignment.
bool KinkFitter::refitRecoTrackAfterReassign(RecoTrack* recoTrackMotherRefit, RecoTrack* recoTrackDaughterRefit,
                                             const RecoTrack* recoTrackMother, const RecoTrack* recoTrackDaughter)
{

  // initialize fitter
  TrackFitter trackFitterDAF;
  // fit the new tracks
  trackFitterDAF.fit(*recoTrackMotherRefit);
  trackFitterDAF.fit(*recoTrackDaughterRefit);

  if (!recoTrackMotherRefit->wasFitSuccessful() || !recoTrackDaughterRefit->wasFitSuccessful()) {
    B2DEBUG(29, "Refit of the tracks with reassigned hits failed ");
    return false;
  }

  const genfit::FitStatus* motherTrackFitStatus = recoTrackMother->getTrackFitStatus();
  const genfit::FitStatus* daughterTrackFitStatus = recoTrackDaughter->getTrackFitStatus();
  const double chi2MotherInit = motherTrackFitStatus->getChi2();
  const double chi2DaughterInit = daughterTrackFitStatus->getChi2();
  const double ndfMotherInit = motherTrackFitStatus->getNdf();
  const double ndfDaughterInit = daughterTrackFitStatus->getNdf();
  B2DEBUG(29, "Initial mother fit result, p-value: " << motherTrackFitStatus->getPVal() << ", chi2: " <<
          chi2MotherInit << ", ndf: " << ndfMotherInit);
  B2DEBUG(29, "Initial daughter fit result, p-value: " << daughterTrackFitStatus->getPVal() << ", chi2: " <<
          chi2DaughterInit << ", ndf: " << ndfDaughterInit);

  const genfit::FitStatus* motherNewTrackFitStatus = recoTrackMotherRefit->getTrackFitStatus();
  const genfit::FitStatus* daughterNewTrackFitStatus = recoTrackDaughterRefit->getTrackFitStatus();
  const double chi2Mother = motherNewTrackFitStatus->getChi2();
  const double chi2Daughter = daughterNewTrackFitStatus->getChi2();
  const double ndfMother = motherNewTrackFitStatus->getNdf();
  const double ndfDaughter = daughterNewTrackFitStatus->getNdf();
  B2DEBUG(29, "New mother fit result, p-value: " << motherNewTrackFitStatus->getPVal() << ", chi2: " <<
          chi2Mother << ", ndf: " << ndfMother);
  B2DEBUG(29, "New daughter fit result, p-value: " << daughterNewTrackFitStatus->getPVal() << ", chi2: " <<
          chi2Daughter << ", ndf: " << ndfDaughter);

  // Fit is assumed to be successful if it improves sum of the chi2 divided by sum of ndf of two tracks
  if ((chi2Mother + chi2Daughter) / (ndfMother + ndfDaughter) < (chi2MotherInit + chi2DaughterInit) /
      (ndfMotherInit + ndfDaughterInit))
    return true;
  else
    return false;

}

/// Flip and refit the daughter track.
RecoTrack* KinkFitter::copyRecoTrackForFlipAndRefit(const RecoTrack* recoTrack,
                                                    const ROOT::Math::XYZVector& momentumSeed,
                                                    const ROOT::Math::XYZVector& positionSeed,
                                                    const double& timeSeed)
{
  // copy recoTracks to a separate StoreArray using seed information
  RecoTrack* newRecoTrack = m_copiedRecoTracks.appendNew(positionSeed, -momentumSeed,
                                                         static_cast<short>(-recoTrack->getChargeSeed()),
                                                         recoTrack->getStoreArrayNameOfPXDHits(),
                                                         recoTrack->getStoreArrayNameOfSVDHits(),
                                                         recoTrack->getStoreArrayNameOfCDCHits(),
                                                         recoTrack->getStoreArrayNameOfBKLMHits(),
                                                         recoTrack->getStoreArrayNameOfEKLMHits(),
                                                         recoTrack->getStoreArrayNameOfRecoHitInformation());
  newRecoTrack->setTimeSeed(timeSeed);
  newRecoTrack->setSeedCovariance(recoTrack->getSeedCovariance());
  newRecoTrack->addHitsFromRecoTrack(recoTrack, 0, true);

  // initialize fitter
  TrackFitter trackFitterKF;
  // DAF fitter usually works badly with flipped tracks, so kalmanFitter is used instead
  std::shared_ptr<genfit::KalmanFitterRefTrack> kalmanFitter = std::make_shared<genfit::KalmanFitterRefTrack>();
  kalmanFitter->setMinIterations(static_cast<unsigned int>(3));
  kalmanFitter->setMaxIterations(static_cast<unsigned int>(10));
  kalmanFitter->setMaxFailedHits(static_cast<unsigned int>(5));
  trackFitterKF.resetFitter(kalmanFitter);
  // fit the new track
  trackFitterKF.fit(*newRecoTrack);

  return newRecoTrack;
}

/// Refit the daughter track blocking hits if required.
RecoTrack* KinkFitter::copyRecoTrackForRefit(const RecoTrack* recoTrack,
                                             const ROOT::Math::XYZVector& momentumSeed,
                                             const ROOT::Math::XYZVector& positionSeed,
                                             const double& timeSeed,
                                             const bool blockInnerStereoHits, const bool useAnotherFitter)
{

  // copy recoTracks to a separate StoreArray using seed information
  RecoTrack* newRecoTrack = m_copiedRecoTracks.appendNew(positionSeed, momentumSeed,
                                                         static_cast<short>(recoTrack->getChargeSeed()),
                                                         recoTrack->getStoreArrayNameOfPXDHits(),
                                                         recoTrack->getStoreArrayNameOfSVDHits(),
                                                         recoTrack->getStoreArrayNameOfCDCHits(),
                                                         recoTrack->getStoreArrayNameOfBKLMHits(),
                                                         recoTrack->getStoreArrayNameOfEKLMHits(),
                                                         recoTrack->getStoreArrayNameOfRecoHitInformation());
  newRecoTrack->setTimeSeed(timeSeed);
  newRecoTrack->setSeedCovariance(recoTrack->getSeedCovariance());
  newRecoTrack->addHitsFromRecoTrack(recoTrack);

  // block the hits in the first stereo layer and all before (leave at least 6 hits for fit)
  // (usually, wrong assignment of first stereo layer is responsible for wrong z coordinate)
  if (blockInnerStereoHits) {
    bool passedStereo = false;
    auto newCDCHitRefit = newRecoTrack->getSortedCDCHitList();
    for (int daughterCDCHitIndex = 0; daughterCDCHitIndex < static_cast<int>(newCDCHitRefit.size()) - 6; ++daughterCDCHitIndex) {
      if (!passedStereo && (newCDCHitRefit[daughterCDCHitIndex]->getISuperLayer() % 2 != 0))
        passedStereo = true;
      if (passedStereo && (newCDCHitRefit[daughterCDCHitIndex]->getISuperLayer() % 2 == 0))
        break;
      auto recoHitInfo = newRecoTrack->getRecoHitInformation(newCDCHitRefit[daughterCDCHitIndex]);
      recoHitInfo->setUseInFit(false);
    }
  }

  // fit the new track
  // initialize fitter
  TrackFitter trackFitter;
  // if useAnotherFitter true, set ordinary KalmanFilter (for filterFlag 6 it performs better than DAF)
  if (useAnotherFitter) {
    std::shared_ptr<genfit::KalmanFitterRefTrack> kalmanFitter = std::make_shared<genfit::KalmanFitterRefTrack>();
    kalmanFitter->setMinIterations(static_cast<unsigned int>(3));
    kalmanFitter->setMaxIterations(static_cast<unsigned int>(10));
    kalmanFitter->setMaxFailedHits(static_cast<unsigned int>(5));
    trackFitter.resetFitter(kalmanFitter);

    trackFitter.fit(*newRecoTrack);
  } else {
    trackFitter.fit(*newRecoTrack);
  }

  return newRecoTrack;
}

/// check if the refit of daughter track for filterFlag 6 (mother end point and daughter Helix extrapolation close in 2D)
/// improves the 3D distance between mother and daughter
bool KinkFitter::isRefitImproveFilter6(const RecoTrack* recoTrackDaughterRefit, const ROOT::Math::XYZVector& motherPosLast)
{
  // get the values near the mother last point
  ROOT::Math::XYZVector daughterPosClosestToMotherPosLast = ROOT::Math::XYZVector(
                                                              recoTrackDaughterRefit->getMeasuredStateOnPlaneFromFirstHit().getPos());
  ROOT::Math::XYZVector daughterMomClosestToMotherPosLast = ROOT::Math::XYZVector(
                                                              recoTrackDaughterRefit->getMeasuredStateOnPlaneFromFirstHit().getMom());
  const double Bz = BFieldManager::getFieldInTesla(daughterPosClosestToMotherPosLast).Z();
  // daughter Helix with move to mother last point
  Helix daughterHelixClosestToMotherPosLast(daughterPosClosestToMotherPosLast,
                                            daughterMomClosestToMotherPosLast,
                                            static_cast<short>(recoTrackDaughterRefit->getTrackFitStatus()->getCharge()),
                                            Bz);
  daughterHelixClosestToMotherPosLast.passiveMoveBy(motherPosLast);

  // check if the 3D distance passes loose criteria (in default track fit used in KinkFinderModule this test is failed)
  return ((daughterHelixClosestToMotherPosLast.getD0() * daughterHelixClosestToMotherPosLast.getD0() +
           daughterHelixClosestToMotherPosLast.getZ0() * daughterHelixClosestToMotherPosLast.getZ0()) <
          m_precutDistance * m_precutDistance);
}

/// combine daughter and mother tracks and fit the result to check for clones
unsigned int KinkFitter::combineTracksAndFit(const Track* trackMother, const Track* trackDaughter)
{
  RecoTrack* recoTrackMother = trackMother->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackDaughter = trackDaughter->getRelated<RecoTrack>(m_recoTracksName);

  // create a combined track by reassigning all daughter track hits to mother track
  RecoTrack* recoTrackCombinedRefit = copyRecoTrackAndReassignCDCHits(recoTrackMother,
                                      recoTrackDaughter, true, -recoTrackDaughter->getNumberOfCDCHits());

  // initialize fitter
  TrackFitter trackFitterDAF;
  // fit the new track
  trackFitterDAF.fit(*recoTrackCombinedRefit);

  // return 19 if the track fit failed
  if (!recoTrackCombinedRefit->wasFitSuccessful()) {
    B2DEBUG(29, "Refit of the combined track failed ");
    return 19;
  }

  // fit results of daughter, mother, and new combined tracks
  const genfit::FitStatus* motherTrackFitStatus = recoTrackMother->getTrackFitStatus();
  const genfit::FitStatus* daughterTrackFitStatus = recoTrackDaughter->getTrackFitStatus();
  const genfit::FitStatus* combinedTrackFitStatus = recoTrackCombinedRefit->getTrackFitStatus();

  B2DEBUG(29, "Initial mother fit result, p-value: " << motherTrackFitStatus->getPVal() << ", ndf: " <<
          motherTrackFitStatus->getNdf());
  B2DEBUG(29, "Initial daughter fit result, p-value: " << daughterTrackFitStatus->getPVal() << ", ndf: " <<
          daughterTrackFitStatus->getNdf());
  B2DEBUG(29, "Combined track fit result, p-value: " << combinedTrackFitStatus->getPVal() << ", ndf: " <<
          combinedTrackFitStatus->getNdf());

  // return 18 if the combined track has NDF less than mother track
  if (combinedTrackFitStatus->getNdf() < motherTrackFitStatus->getNdf())
    return 18;

  // filling bits according to the fit result of the combined track
  const int motherFlag = (combinedTrackFitStatus->getPVal() > motherTrackFitStatus->getPVal());
  const int daughterFlag = 2 * (combinedTrackFitStatus->getPVal() > daughterTrackFitStatus->getPVal());
  const int daughterNdfFlag = 4 * (combinedTrackFitStatus->getNdf() > daughterTrackFitStatus->getNdf());
  const int pValueFlag = 8 * (combinedTrackFitStatus->getPVal() > 0.0000001); // almost 5 sigma

  return motherFlag + daughterFlag + daughterNdfFlag + pValueFlag;
}

/// Create a RecoTrack in a separate StoreArray based on one to be split
RecoTrack* KinkFitter::copyRecoTrackAndSplit(const RecoTrack* splitRecoTrack,
                                             const bool motherFlag, const unsigned int delta)
{

  // sorted list of CDC hits of track to be split
  const auto splitCDCHit = splitRecoTrack->getSortedCDCHitList();

  // seeds used for a copy RecoTrack
  ROOT::Math::XYZVector positionSeed(0, 0, 0);
  ROOT::Math::XYZVector momentumSeed(0, 0, 0);
  const short chargeSeed = splitRecoTrack->getTrackFitStatus()->getCharge();
  double timeSeed = 0;
  if (motherFlag) {
    positionSeed = ROOT::Math::XYZVector(splitRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getPos());
    momentumSeed = ROOT::Math::XYZVector(splitRecoTrack->getMeasuredStateOnPlaneFromFirstHit().getMom());
    timeSeed = splitRecoTrack->getCardinalRepresentation()->getTime(
                 splitRecoTrack->getMeasuredStateOnPlaneFromFirstHit());
  } else {
    positionSeed = ROOT::Math::XYZVector(splitRecoTrack->getMeasuredStateOnPlaneFromLastHit().getPos());
    momentumSeed = ROOT::Math::XYZVector(splitRecoTrack->getMeasuredStateOnPlaneFromLastHit().getMom());
    timeSeed = splitRecoTrack->getCardinalRepresentation()->getTime(
                 splitRecoTrack->getMeasuredStateOnPlaneFromLastHit());
  }

  // copy recoTrack to a separate StoreArray using seed information
  RecoTrack* recoTrack = m_copiedRecoTracks.appendNew(positionSeed, momentumSeed, chargeSeed,
                                                      splitRecoTrack->getStoreArrayNameOfPXDHits(),
                                                      splitRecoTrack->getStoreArrayNameOfSVDHits(),
                                                      splitRecoTrack->getStoreArrayNameOfCDCHits(),
                                                      splitRecoTrack->getStoreArrayNameOfBKLMHits(),
                                                      splitRecoTrack->getStoreArrayNameOfEKLMHits(),
                                                      splitRecoTrack->getStoreArrayNameOfRecoHitInformation());
  recoTrack->setTimeSeed(timeSeed);
  recoTrack->setSeedCovariance(splitRecoTrack->getSeedCovariance());

  if (motherFlag) {
    // copy PXD hits (we have checked in KinkFinderModule that there no PXD hits on the other side of the track)
    for (const auto* pxdHit : splitRecoTrack->getPXDHitList()) {
      auto recoHitInfo = splitRecoTrack->getRecoHitInformation(pxdHit);
      recoTrack->addPXDHit(pxdHit, recoHitInfo->getSortingParameter(),
                           recoHitInfo->getFoundByTrackFinder());
    }

    // copy SVD hits (we have checked in KinkFinderModule that there no SVD hits on the other side of the track)
    for (const auto* svdHit : splitRecoTrack->getSVDHitList()) {
      auto recoHitInfo = splitRecoTrack->getRecoHitInformation(svdHit);
      recoTrack->addSVDHit(svdHit, recoHitInfo->getSortingParameter(),
                           recoHitInfo->getFoundByTrackFinder());
    }


    // copy CDC hits with respect to reassignment
    for (size_t splitCDCHitIndex = 0; splitCDCHitIndex < splitCDCHit.size() - delta; ++splitCDCHitIndex) {
      auto recoHitInfo = splitRecoTrack->getRecoHitInformation(splitCDCHit[splitCDCHitIndex]);
      recoTrack->addCDCHit(splitCDCHit[splitCDCHitIndex],
                           recoHitInfo->getSortingParameter(),
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }

    // KLM hits are never assigned to mother track during splitting

  } else {

    // copy CDC hits with respect to reassignment
    int sortingParameterOffset = splitRecoTrack->getRecoHitInformation(splitCDCHit[splitCDCHit.size()
                                 - delta])->getSortingParameter();
    for (size_t splitCDCHitIndex = splitCDCHit.size() - delta; splitCDCHitIndex < splitCDCHit.size(); ++splitCDCHitIndex) {
      auto recoHitInfo = splitRecoTrack->getRecoHitInformation(splitCDCHit[splitCDCHitIndex]);
      recoTrack->addCDCHit(splitCDCHit[splitCDCHitIndex],
                           splitCDCHitIndex - sortingParameterOffset,
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }

    // copy BKLM hits
    for (const auto* bklmHit : splitRecoTrack->getBKLMHitList()) {
      auto recoHitInfo = splitRecoTrack->getRecoHitInformation(bklmHit);
      recoTrack->addBKLMHit(bklmHit, recoHitInfo->getSortingParameter() - sortingParameterOffset,
                            recoHitInfo->getFoundByTrackFinder());
    }

    // copy EKLM hits
    for (const auto* eklmHit : splitRecoTrack->getEKLMHitList()) {
      auto recoHitInfo = splitRecoTrack->getRecoHitInformation(eklmHit);
      recoTrack->addEKLMHit(eklmHit, recoHitInfo->getSortingParameter() - sortingParameterOffset,
                            recoHitInfo->getFoundByTrackFinder());
    }
  }

  return recoTrack;

}

/// Split track into two based on |chi2/ndf - 1|
bool KinkFitter::splitRecoTrack(const RecoTrack* recoTrackSplit, short& recoTrackIndexMother, short& recoTrackIndexDaughter)
{
  // initialize fitter
  TrackFitter trackFitterDAF;

  // initial default value for the |chi2/ndf - 1|
  constexpr double defaultChi2NdfRatio = std::numeric_limits<double>::max();

  // enum for the splitting points used in the best splitting point search
  enum {
    c_outerEdge, c_middlePoint, c_innerEdge
  };

  B2DEBUG(29, "Start splitting");

  // fit result of the initial track to be split
  const genfit::FitStatus* splitTrackFitStatus = recoTrackSplit->getTrackFitStatus();
  const double ndfSplit = splitTrackFitStatus->getNdf();
  const double chi2Split = splitTrackFitStatus->getChi2();
  const double chi2NdfRatioSplit = fabs(chi2Split / ndfSplit - 1);
  B2DEBUG(29, "Initial fit result, p-value: " <<  splitTrackFitStatus->getPVal() << ", chi2: " <<
          chi2Split << ", ndf: " << ndfSplit);

  // number of SVD+CDC hits of the initial track to be split
  const unsigned int numberSVDCDCHitsSplit = recoTrackSplit->getNumberOfCDCHits() + recoTrackSplit->getNumberOfSVDHits();

  // Three initial points for the binary search. Since the main target are tracks with mixed fractions of hits from two
  // wrongly combined tracks between 30-70%, the points are chosen between 20-80%.
  // The first point is for 80% (20%) of hits assigned to mother (daughter), the second one is for middle 50% (50%),
  // and the last one is for 20% (80%), respectively.
  // Due to the logic of the copyRecoTrackAndSplit function, the hits position in splitHitNumber
  // is counted from the end of the track to split (the index 0 is for c_outerEdge, index 1 is for c_middlePoint,
  // and index 2 is for c_innerEdge).
  std::array<unsigned int, 3> splitHitNumber = {
    static_cast<unsigned int>(numberSVDCDCHitsSplit * 0.2),
    static_cast<unsigned int>(numberSVDCDCHitsSplit * 0.5),
    static_cast<unsigned int>(numberSVDCDCHitsSplit * 0.8)
  };

  // initialization of arrays of the created mother and daughter RecoTracks and |chi2/ndf - 1| for them
  // chi2/ndf ratios for resulting tracks are calculated as (chi2_mother + chi2_daughter)/(ndf_mother + ndf_daughter)
  std::array<RecoTrack*, 3> recoTrackMother = {nullptr, nullptr, nullptr};
  std::array<RecoTrack*, 3> recoTrackDaughter = {nullptr, nullptr, nullptr};
  std::array<double, 3> chi2NdfRatio = {defaultChi2NdfRatio, defaultChi2NdfRatio, defaultChi2NdfRatio};

  // initial splitting and fitting of the resulting tracks
  for (size_t splitHitNumberIndex = 0; splitHitNumberIndex < splitHitNumber.size(); ++splitHitNumberIndex) {
    // if there is not enough hits for fit of at least one resulting track, continue
    if (splitHitNumber[splitHitNumberIndex] < 5 ||
        (numberSVDCDCHitsSplit - splitHitNumber[splitHitNumberIndex]) < 5) continue;

    // Splitting is based only on the CDC hits, so all VXD hits are assigned to mother. If the splitting number exceeds
    // number of CDC hits, use the latter one.
    if (splitHitNumber[splitHitNumberIndex] > recoTrackSplit->getNumberOfCDCHits())
      splitHitNumber[splitHitNumberIndex] = recoTrackSplit->getNumberOfCDCHits();

    B2DEBUG(29, "Initial splitting hit number for point " << splitHitNumberIndex << ": " <<
            splitHitNumber[splitHitNumberIndex]);

    // creation of split RecoTracks
    recoTrackMother[splitHitNumberIndex] = copyRecoTrackAndSplit(recoTrackSplit, true, splitHitNumber[splitHitNumberIndex]);
    recoTrackDaughter[splitHitNumberIndex] = copyRecoTrackAndSplit(recoTrackSplit, false, splitHitNumber[splitHitNumberIndex]);

    // fit of the RecoTracks
    trackFitterDAF.fit(*(recoTrackMother[splitHitNumberIndex]));
    trackFitterDAF.fit(*(recoTrackDaughter[splitHitNumberIndex]));

    // if the fit failed, skip this track pair
    if (!recoTrackMother[splitHitNumberIndex]->wasFitSuccessful() ||
        !recoTrackDaughter[splitHitNumberIndex]->wasFitSuccessful()) continue;

    // fit result of the split track pair
    const genfit::FitStatus* motherNewTrackFitStatus = recoTrackMother[splitHitNumberIndex]->getTrackFitStatus();
    const genfit::FitStatus* daughterNewTrackFitStatus = recoTrackDaughter[splitHitNumberIndex]->getTrackFitStatus();
    const double ndfMotherTmp = motherNewTrackFitStatus->getNdf();
    const double ndfDaughterTmp = daughterNewTrackFitStatus->getNdf();
    const double chi2MotherTmp = motherNewTrackFitStatus->getChi2();
    const double chi2DaughterTmp = daughterNewTrackFitStatus->getChi2();
    const double chi2NdfRatioTmp = fabs((chi2MotherTmp + chi2DaughterTmp) / (ndfMotherTmp + ndfDaughterTmp) - 1);
    B2DEBUG(29, "Mother fit result for initial point " << splitHitNumberIndex << ", p-value: " <<
            motherNewTrackFitStatus->getPVal() << ", chi2: " << chi2MotherTmp << ", ndf: " << ndfMotherTmp);
    B2DEBUG(29, "Daughter fit result for initial point " << splitHitNumberIndex << ", p-value: " <<
            daughterNewTrackFitStatus->getPVal() << ", chi2: " << chi2DaughterTmp << ", ndf: " << ndfDaughterTmp);
    B2DEBUG(29, "|Chi2/NDF - 1| for initial point " << splitHitNumberIndex << ": " <<
            chi2NdfRatioTmp << ", and for initial track: " << chi2NdfRatioSplit);

    // if the resulting |chi2/ndf - 1| is bigger than one for initial track, skip this track pair
    if (chi2NdfRatioTmp > chi2NdfRatioSplit) continue;

    chi2NdfRatio[splitHitNumberIndex] = chi2NdfRatioTmp;
  }

  // Find the point with minimal difference of chi2/ndf ratio with unity using binary search.
  // Compare the |chi2/ndf - 1| result for edge points and choose the interval between middle and the better edge.
  // Find a middle point in the chosen interval, split the track, and fit the result.
  // Return to the comparison.
  // In a special case when both edges fit failed (the values of |chi2/ndf - 1| are equal),
  // check if the middle point is fitted. If it is, use it as a result. If it is not, return false.
  // Try five iterations (usually, enough to converge). If the middle point equals one of the edge, the result has
  // converged earlier. Break the loop and proceed the execution of the function.
  for (unsigned int searchIteration = 0; searchIteration < 5; ++searchIteration) {
    B2DEBUG(29, "Splitting iteration " << searchIteration << "; hit numbers: " << splitHitNumber[c_outerEdge] << ", " <<
            splitHitNumber[c_middlePoint] << ", " << splitHitNumber[c_innerEdge]);

    if (chi2NdfRatio[c_outerEdge] < chi2NdfRatio[c_innerEdge]) { // choose outer interval

      // replace the inner edge with the middle point
      splitHitNumber[c_innerEdge] = splitHitNumber[c_middlePoint];
      chi2NdfRatio[c_innerEdge] = chi2NdfRatio[c_middlePoint];
      recoTrackMother[c_innerEdge] = recoTrackMother[c_middlePoint];
      recoTrackDaughter[c_innerEdge] = recoTrackDaughter[c_middlePoint];
      // find a new middle point in the outer interval, and fill the other arrays with the default values
      splitHitNumber[c_middlePoint] = (splitHitNumber[c_innerEdge] + splitHitNumber[c_outerEdge]) / 2;
      chi2NdfRatio[c_middlePoint] = defaultChi2NdfRatio;
      recoTrackMother[c_middlePoint] = nullptr;
      recoTrackDaughter[c_middlePoint] = nullptr;

      B2DEBUG(29, "Outer interval is chosen; hit numbers: " << splitHitNumber[c_outerEdge] << ", " <<
              splitHitNumber[c_middlePoint] << ", " << splitHitNumber[c_innerEdge]);

    } else if (chi2NdfRatio[c_outerEdge] > chi2NdfRatio[c_innerEdge]) { // choose inner interval

      // replace the outer edge with the middle point
      splitHitNumber[c_outerEdge] = splitHitNumber[c_middlePoint];
      chi2NdfRatio[c_outerEdge] = chi2NdfRatio[c_middlePoint];
      recoTrackMother[c_outerEdge] = recoTrackMother[c_middlePoint];
      recoTrackDaughter[c_outerEdge] = recoTrackDaughter[c_middlePoint];
      // find a new middle point in the inner interval, and fill the other arrays with the default values
      splitHitNumber[c_middlePoint] = (splitHitNumber[c_innerEdge] + splitHitNumber[c_outerEdge]) / 2;
      chi2NdfRatio[c_middlePoint] = defaultChi2NdfRatio;
      recoTrackMother[c_middlePoint] = nullptr;
      recoTrackDaughter[c_middlePoint] = nullptr;

      B2DEBUG(29, "Inner interval is chosen; hit numbers: " << splitHitNumber[c_outerEdge] << ", " <<
              splitHitNumber[c_middlePoint] << ", " << splitHitNumber[c_innerEdge]);

    } else if (chi2NdfRatio[c_middlePoint] == defaultChi2NdfRatio) { // if none of the point was fitted successfully, return false

      B2DEBUG(29, "Track splitting failed");
      return false;

    } else { // if both edges fits have the same results (their fit failed), return the middle one

      B2DEBUG(29, "Middle point is chosen; hit numbers: " << splitHitNumber[c_outerEdge] << ", " <<
              splitHitNumber[c_middlePoint] << ", " << splitHitNumber[c_innerEdge]);
      // return through references indexes of the mother and daughter tracks
      recoTrackIndexMother = recoTrackMother[c_middlePoint]->getArrayIndex();
      recoTrackIndexDaughter = recoTrackDaughter[c_middlePoint]->getArrayIndex();
      return true;

    }

    // stop the iterations if the all possibilities were tried
    if (splitHitNumber[c_middlePoint] == splitHitNumber[c_outerEdge] ||
        splitHitNumber[c_middlePoint] == splitHitNumber[c_innerEdge]) break;

    // if there is not enough hits for fit of at least one resulting track, continue
    if (splitHitNumber[c_middlePoint] < 5 ||
        (numberSVDCDCHitsSplit - splitHitNumber[c_middlePoint]) < 5) continue;

    // creation of split RecoTracks in the new middle point
    recoTrackMother[c_middlePoint] = copyRecoTrackAndSplit(recoTrackSplit, true, splitHitNumber[c_middlePoint]);
    recoTrackDaughter[c_middlePoint] = copyRecoTrackAndSplit(recoTrackSplit, false, splitHitNumber[c_middlePoint]);

    // fit of the mother and daughter RecoTracks in the new middle point
    trackFitterDAF.fit(*(recoTrackMother[c_middlePoint]));
    trackFitterDAF.fit(*(recoTrackDaughter[c_middlePoint]));

    // if the fit failed, skip this track pair
    if (!recoTrackMother[c_middlePoint]->wasFitSuccessful() ||
        !recoTrackDaughter[c_middlePoint]->wasFitSuccessful()) continue;

    // fit result of the split track pair
    const genfit::FitStatus* motherNewTrackFitStatus = recoTrackMother[c_middlePoint]->getTrackFitStatus();
    const genfit::FitStatus* daughterNewTrackFitStatus = recoTrackDaughter[c_middlePoint]->getTrackFitStatus();
    const double ndfMotherTmp = motherNewTrackFitStatus->getNdf();
    const double ndfDaughterTmp = daughterNewTrackFitStatus->getNdf();
    const double chi2MotherTmp = motherNewTrackFitStatus->getChi2();
    const double chi2DaughterTmp = daughterNewTrackFitStatus->getChi2();
    const double chi2NdfRatioTmp = fabs((chi2MotherTmp + chi2DaughterTmp) / (ndfMotherTmp + ndfDaughterTmp) - 1);

    B2DEBUG(29, "Mother fit result for approach " << searchIteration << ", p-value: " <<
            motherNewTrackFitStatus->getPVal() << ", chi2: " << chi2MotherTmp << ", ndf: " << ndfMotherTmp);
    B2DEBUG(29, "Daughter fit result for approach " << searchIteration << ", p-value: " <<
            daughterNewTrackFitStatus->getPVal() << ", chi2: " << chi2DaughterTmp << ", ndf: " << ndfDaughterTmp);
    B2DEBUG(29, "Chi2/NDF for approach " << searchIteration << ": " <<
            chi2NdfRatioTmp << ", and for initial track: " << chi2NdfRatioSplit);

    // if the resulting chi2/ndf ratio is bigger than one for initial track, skip this track pair
    if (chi2NdfRatioTmp > chi2NdfRatioSplit) continue;

    chi2NdfRatio[c_middlePoint] = chi2NdfRatioTmp;
  }

  // find the minimal difference of chi2/ndf ratio with unity among left points
  std::array<double, 3>::iterator minChi2NdfRatioResult = std::min_element(chi2NdfRatio.begin(), chi2NdfRatio.end());
  const size_t minIndex = std::distance(chi2NdfRatio.begin(), minChi2NdfRatioResult);


  B2DEBUG(29, "End of splitting with |chi2/ndf - 1| for point 1:" << chi2NdfRatio[c_outerEdge] <<
          "; 2:" << chi2NdfRatio[c_middlePoint] << "; 3:" << chi2NdfRatio[c_innerEdge]);
  B2DEBUG(29, "Hit numbers for point 1:" << splitHitNumber[c_outerEdge] <<
          "; 2:" << splitHitNumber[c_middlePoint] << "; 3:" << splitHitNumber[c_innerEdge]);
  B2DEBUG(29, "Min index: " << minIndex);

  // return through references indexes of the mother and daughter tracks for the best splitting point
  recoTrackIndexMother = recoTrackMother[minIndex]->getArrayIndex();
  recoTrackIndexDaughter = recoTrackDaughter[minIndex]->getArrayIndex();

  return true;

}

/// Fit and store kink.
/// If the fitterMode requires, tries to reassign hits between daughter and mother tracks.
/// If the fitterMode requires, tries to flip and refit the tracks.
bool KinkFitter::fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag)
{

  // Existence of corresponding RecoTrack already checked at the module level;
  RecoTrack* recoTrackMother = trackMother->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackDaughter = trackDaughter->getRelated<RecoTrack>(m_recoTracksName);

  // Track splitting (filterFlag 7, 8, and 9)
  if (filterFlag >= 7 && filterFlag <= 9) {
    short recoTrackIndexMother = -1;
    short recoTrackIndexDaughter = -1;
    if (!splitRecoTrack(recoTrackMother, recoTrackIndexMother, recoTrackIndexDaughter))
      return false;
    recoTrackMother = m_copiedRecoTracks[recoTrackIndexMother];
    recoTrackDaughter = m_copiedRecoTracks[recoTrackIndexDaughter];
  }

  // Tracks selected with filterFlag from 4 to 6 are selected by 2D distance cut assuming bad z coordinate.
  // Initial refit is required for such tracks in the majority of the cases.
  // If the refit successful, use new RecoTrack for the vertex fit.

  // Initial refit daughter track for filterFlag 4 (mother end point and daughter start point close in 2D) and
  // filterFlag 6 (mother end point and daughter Helix extrapolation close in 2D), which do not require flipping.
  bool refitBadFlag = false;
  if (filterFlag == 4 || filterFlag == 6) {
    B2DEBUG(29, "Try to do initial refit of daughter track for filterFlag " << filterFlag);

    // initialize seeds for the refit
    // position of the last mother state
    ROOT::Math::XYZVector motherPosLast = ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos());
    // use mother last state time as a seed for the daughter track
    double timeSeedDaughterRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                     recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the first hit for a momentum seed
    ROOT::Math::XYZVector momSeedDaughterRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getMom());

    // initialize refit conditions
    // remove hits until the first stereo layer is passed
    bool blockInnerStereoHits = false;
    if (filterFlag == 4) blockInnerStereoHits = true;
    // use ordinary KalmanFilter
    bool anotherFitter = false;
    if (filterFlag == 6) anotherFitter = true;

    // create a copy of the daughter track and refit it
    RecoTrack* recoTrackDaughterRefit = copyRecoTrackForRefit(recoTrackDaughter, momSeedDaughterRefit,
                                                              motherPosLast, timeSeedDaughterRefit,
                                                              blockInnerStereoHits, anotherFitter);

    // if the new track fit is successful, and in addition, the distance for the filterFlag 6
    // (mother end point and daughter Helix extrapolation close in 2D) is improved,
    // use it for the vertex fit
    if (recoTrackDaughterRefit->wasFitSuccessful()) {
      if (filterFlag == 4 ||
          (filterFlag == 6 && isRefitImproveFilter6(recoTrackDaughterRefit, motherPosLast)))
        recoTrackDaughter = recoTrackDaughterRefit;
      B2DEBUG(29, "Initial refit successful");
      refitBadFlag = true;
    }
  }

  // Flip and refit filterFlag 5 (mother end point and daughter end point close in 2D).
  if (m_kinkFitterModeFlipAndRefit && (filterFlag == 5)) {
    B2DEBUG(29, "Try to do initial flip and refit of daughter track for filterFlag " << filterFlag);

    // initialize seeds for the refit
    // use position of the last mother state as a seed for the daughter track
    ROOT::Math::XYZVector motherPosLast = ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos());
    // use mother last state time as a seed for the daughter track
    double timeSeedDaughterFlipAndRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                            recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the last hit for a momentum seed
    ROOT::Math::XYZVector momSeedDaughterFlipAndRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromLastHit().getMom());

    // create a copy of the daughter track, flipped and refitted
    RecoTrack* recoTrackDaughterFlipAndRefit = copyRecoTrackForFlipAndRefit(recoTrackDaughter,
                                               motherPosLast, momSeedDaughterFlipAndRefit,
                                               timeSeedDaughterFlipAndRefit);

    // if the new track fit is successful, use it for the vertex fit
    if (recoTrackDaughterFlipAndRefit->wasFitSuccessful()) {
      recoTrackDaughter = recoTrackDaughterFlipAndRefit;
      B2DEBUG(29, "Initial flip and refit successful");
    }
  }

  // fitted vertex position
  ROOT::Math::XYZVector vertexPos = ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos());

  // flag to reassign hits, final hit to reassign, and distance at the fitted vertex
  unsigned int reassignHitStatus = 0;
  int finalHitPositionForReassignment = 0;
  double distanceAtVertex = std::numeric_limits<double>::max();

  // Try kink vertex fit. If the fit fails, return false immediately for all except
  // filterFlag 1 (mother end point and daughter start point close in 3D) and
  // filterFlag 3 (mother end point and daughter Helix extrapolation close in 3D).
  bool failedFitFlag = !vertexFitWithRecoTracks(recoTrackMother, recoTrackDaughter, reassignHitStatus, vertexPos, distanceAtVertex,
                                                ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos()));
  if (failedFitFlag && (filterFlag != 1 && filterFlag != 3))
    return false;

  // If the fit fails for filterFlag 1 (mother end point and daughter start point close in 3D),
  // try to refit daughter track blocking the first stereo superlayer.
  if (failedFitFlag && filterFlag == 1) {
    B2DEBUG(29, "Try to do postVertexFit refit of daughter track for filterFlag " << filterFlag);

    // initialize seeds for the refit
    // position of the last mother state
    const ROOT::Math::XYZVector motherPosLast = ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos());
    // use mother last state time as a seed for the daughter track
    const double timeSeedDaughterRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                           recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the first hit for a momentum seed
    const ROOT::Math::XYZVector momSeedDaughterRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getMom());

    // initialize refit conditions
    // remove hits until the first stereo layer is passed
    const bool blockInnerStereoHits = true;
    // do not use ordinary KalmanFilter
    const bool anotherFitter = false;

    // create a copy of the daughter track and refit it
    RecoTrack* recoTrackDaughterRefit = copyRecoTrackForRefit(recoTrackDaughter, momSeedDaughterRefit,
                                                              motherPosLast, timeSeedDaughterRefit,
                                                              blockInnerStereoHits, anotherFitter);

    // if the new track fit is successful, and in addition, the vertex fit is successful,
    // use a new fit and proceed. Otherwise, return false.
    if (recoTrackDaughterRefit->wasFitSuccessful() &&
        vertexFitWithRecoTracks(recoTrackMother, recoTrackDaughterRefit, reassignHitStatus, vertexPos, distanceAtVertex,
                                motherPosLast)) {
      recoTrackDaughter = recoTrackDaughterRefit;
      B2DEBUG(29, "postVertexFit refit successful");
      refitBadFlag = true;
    } else return false;
  }

  // If the daughter track for filterFlag 4 (mother end point and daughter start point close in 2D) or
  // filterFlag 1 (mother end point and daughter start point close in 3D) (if required) was refitted successfully,
  // reassignHitStatus may not be assigned due to blocked hits.
  // In this case, reassignment from daughter to mother is required, so we set it manually.
  if ((filterFlag == 4 || (failedFitFlag && filterFlag == 1)) && (refitBadFlag) && (reassignHitStatus == 0))
    reassignHitStatus |= 0x1;

  // Cache the objects used in vertex fit
  // Mother and daughter States extrapolated to the fitted vertex
  genfit::MeasuredStateOnPlane stMother = m_stMotherBuffer;
  genfit::MeasuredStateOnPlane stDaughter = m_stDaughterBuffer;
  // recoTracks used to fit the vertex
  m_motherKinkRecoTrackCache = recoTrackMother;
  m_daughterKinkRecoTrackCache = recoTrackDaughter;

  // if the corresponding fitterMode is used, try to reassign hits between mother and daughter tracks
  // This is done only for filterFlags, which require mother end point and daughter start point to be close to each other.
  // So filterFlags are 1, 4, and from 7 to 9 (track split).
  if (m_kinkFitterModeHitsReassignment &&
      (filterFlag == 1 || filterFlag == 4  ||
       (filterFlag >= 7 && filterFlag <= 9 && distanceAtVertex > m_vertexDistanceCut)) &&
      (reassignHitStatus != 0)) {
    B2DEBUG(29, "Start of the hits reassignment for filterFlag " << filterFlag);

    // initialize counter for reassigning tries
    unsigned short countReassignTries = 0;

    // variables to store temporary values
    int finalHitPositionForReassignmentTmp = 0;
    double distanceAtVertexTmp = std::numeric_limits<double>::max();
    ROOT::Math::XYZVector vertexPosTmp(vertexPos);
    RecoTrack* recoTrackMotherRefit = nullptr;
    RecoTrack* recoTrackDaughterRefit = nullptr;
    RecoTrack* recoTrackMotherBuffer = recoTrackMother;
    RecoTrack* recoTrackDaughterBuffer = recoTrackDaughter;

    // The number of tries is limited to 3
    while (reassignHitStatus != 0 && countReassignTries < 3) {
      ++countReassignTries;
      B2DEBUG(29, "Try number " << countReassignTries);

      // counter for failed retries
      unsigned short countBadReassignTries = 0;

      // find threshold hit position in the track
      // hit bellow the threshold are to be reassigned
      // positive for daughter hits reassignment to mother, negative vice-versa
      int hitPositionForReassignment = 0;
      if (reassignHitStatus & 0x1) {

        // daughter hits to be reassigned (negative value); direction should be 1
        hitPositionForReassignment = findHitPositionForReassignment(recoTrackDaughterBuffer, vertexPosTmp, 1);

        // test if the number of hits to be reassigned larger than the number of CDC hits in daughter tracks
        // minus number degree of freedom required for the fit
        if (static_cast<unsigned int>(-hitPositionForReassignment + 5) >
            recoTrackDaughterBuffer->getNumberOfCDCHits())
          break;

      } else if (reassignHitStatus & 0x2) {

        // mother hits to be reassigned (positive value); direction should be -1
        hitPositionForReassignment = findHitPositionForReassignment(recoTrackMotherBuffer, vertexPosTmp, -1);

        // test if the number of hits to be reassigned larger than the number of CDC hits in mother tracks
        // minus number degree of freedom required for the fit
        if (static_cast<unsigned int>(hitPositionForReassignment + 5) > recoTrackMotherBuffer->getNumberOfCDCHits()) {

          // if number of SVD hits is enough for the fit, reassign all the CDC hits
          if (recoTrackMotherBuffer->getNumberOfSVDHits() > 5)
            hitPositionForReassignment = recoTrackMotherBuffer->getNumberOfCDCHits();
          else
            break;

        }
      }
      B2DEBUG(29, "Found hit index, starting from which hits are reassigned: " << hitPositionForReassignment);

      // refit of the new tracks can fail when the position is too far
      // try positions closer to the end until reach it
      while (hitPositionForReassignment != 0) {

        // create new RecoTracks with reassigned hits in the separate StoreArray
        recoTrackMotherRefit = copyRecoTrackAndReassignCDCHits(recoTrackMotherBuffer,
                                                               recoTrackDaughterBuffer,
                                                               true, hitPositionForReassignment);
        recoTrackDaughterRefit = copyRecoTrackAndReassignCDCHits(recoTrackMotherBuffer,
                                                                 recoTrackDaughterBuffer,
                                                                 false, hitPositionForReassignment);

        // try to fit new RecoTracks assuming improvement of the result
        // if fit fails, try position closer to the end (no more than 5 tries)
        // if fit is successful, break the loop
        if (!refitRecoTrackAfterReassign(recoTrackMotherRefit, recoTrackDaughterRefit,
                                         recoTrackMother, recoTrackDaughter)) {
          if (hitPositionForReassignment > 0) {
            --hitPositionForReassignment;
          } else {
            ++hitPositionForReassignment;
          }
          ++countBadReassignTries;

          if (countBadReassignTries > 5) hitPositionForReassignment = 0;
        } else
          break;
        B2DEBUG(29, "Refit of the tracks failed, try with smaller hit index: " << hitPositionForReassignment);
      }

      // if hit position reaches end, the trial failed. Continue with default tracks.
      if (hitPositionForReassignment == 0) {
        B2DEBUG(29, "Reassigning of hits and refitting failed");
        break;
      }

      // Try to fit vertex, using previous result as a seed. If the fit fails, continue with current tracks.
      if (!vertexFitWithRecoTracks(recoTrackMotherRefit, recoTrackDaughterRefit, reassignHitStatus,
                                   vertexPosTmp, distanceAtVertexTmp, vertexPosTmp))
        break;
      recoTrackMotherBuffer = recoTrackMotherRefit;
      recoTrackDaughterBuffer = recoTrackDaughterRefit;
      finalHitPositionForReassignmentTmp += hitPositionForReassignment;

      // Remember the result leading to the smallest distance between tracks.
      if (distanceAtVertexTmp < m_vertexDistanceCut) {
        distanceAtVertex = distanceAtVertexTmp;
        vertexPos = vertexPosTmp;
        stMother = m_stMotherBuffer;
        stDaughter = m_stDaughterBuffer;

        m_motherKinkRecoTrackCache = recoTrackMotherRefit;
        m_daughterKinkRecoTrackCache = recoTrackDaughterRefit;
        finalHitPositionForReassignment = finalHitPositionForReassignmentTmp;
      }
    }
  }

  // If the corresponding fitterMode is used, try to flip and refit daughter track for
  // filterFlag 2 (mother end point and daughter end point close in 3D).
  if (m_kinkFitterModeFlipAndRefit && (filterFlag == 2)) {
    B2DEBUG(29, "Try to do postVertexFit flip and refit of daughter track for filterFlag " << filterFlag);

    // variables to store temporary values
    double distanceAtVertexTmp = std::numeric_limits<double>::max();
    ROOT::Math::XYZVector vertexPosTmp(vertexPos);

    // use mother last state time as a seed for the daughter track
    const double timeSeedDaughterFlipAndRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                                  recoTrackMother->getMeasuredStateOnPlaneFromLastHit());

    // use state at the fitted vertex for a momentum seed
    const ROOT::Math::XYZVector momSeedDaughterFlipAndRefit(stDaughter.getMom());

    // create a copy of the daughter track, flipped and refitted
    RecoTrack* recoTrackDaughterFlipAndRefit = copyRecoTrackForFlipAndRefit(recoTrackDaughter,
                                               vertexPos, momSeedDaughterFlipAndRefit,
                                               timeSeedDaughterFlipAndRefit);

    // if the vertex fit is successful and the result is improved, store it
    if (recoTrackDaughterFlipAndRefit->wasFitSuccessful() &&
        vertexFitWithRecoTracks(recoTrackMother, recoTrackDaughterFlipAndRefit, reassignHitStatus,
                                vertexPosTmp, distanceAtVertexTmp, vertexPosTmp))
      if (distanceAtVertexTmp < distanceAtVertex) {
        distanceAtVertex = distanceAtVertexTmp;
        vertexPos = vertexPosTmp;
        stMother = m_stMotherBuffer;
        stDaughter = m_stDaughterBuffer;
        m_daughterKinkRecoTrackCache = recoTrackDaughterFlipAndRefit;

        B2DEBUG(29, "postVertexFit flip and refit successful");
      }
  }

  // Try to refit daughter track for filterFlag 3 (mother end point and daughter Helix extrapolation close in 3D).
  // If it improves the vertex fit, which might even fail before, use a new result.
  if (filterFlag == 3) {
    B2DEBUG(29, "Try to do postVertexFit refit of daughter track for filterFlag " << filterFlag);

    // initialize seeds for the refit
    // position of the last mother state and the first daughter state
    const ROOT::Math::XYZVector motherPosLast = ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos());
    // use mother last state time as a seed for the daughter track
    const double timeSeedDaughterRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                           recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the first hit for a momentum seed
    const ROOT::Math::XYZVector momSeedDaughterRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getMom());

    // variables to store temporary values
    double distanceAtVertexTmp = std::numeric_limits<double>::max();
    ROOT::Math::XYZVector vertexPosTmp(vertexPos);

    // initialize refit conditions
    // do not remove hits until the first stereo layer is passed
    const bool blockInnerStereoHits = false;
    // use ordinary KalmanFilter
    const bool anotherFitter = true;

    // create a copy of the daughter track and refit it
    RecoTrack* recoTrackDaughterRefit = copyRecoTrackForRefit(recoTrackDaughter, momSeedDaughterRefit,
                                                              motherPosLast, timeSeedDaughterRefit,
                                                              blockInnerStereoHits, anotherFitter);

    // if the vertex fit is successful and the result is improved, store it
    if (recoTrackDaughterRefit->wasFitSuccessful() &&
        vertexFitWithRecoTracks(recoTrackMother, recoTrackDaughterRefit, reassignHitStatus,
                                vertexPosTmp, distanceAtVertexTmp, vertexPosTmp)) {
      if ((distanceAtVertexTmp < distanceAtVertex) || failedFitFlag) {
        distanceAtVertex = distanceAtVertexTmp;
        vertexPos = vertexPosTmp;
        stMother = m_stMotherBuffer;
        stDaughter = m_stDaughterBuffer;
        m_daughterKinkRecoTrackCache = recoTrackDaughterRefit;

        B2DEBUG(29, "postVertexFit refit successful");
      }
    } else if (failedFitFlag) return false;
  }

  B2DEBUG(29, "Distance between tracks at fitted kink vertex " << distanceAtVertex);
  B2DEBUG(29, "Radius of the kink vertex " << vertexPos.Rho());
  B2DEBUG(29, "Number of reassigned hits " << finalHitPositionForReassignment);

  // for analysis purposes, there is no need to distinguish kinks selected with some filters,
  // so we rearrange them to simplify the output
  // value 1 is assigned to track pairs which have close endpoints
  // value 2 is assigned to track pairs which have missing layers between their endpoints,
  // so the Helix extrapolation was used
  // value 3-5 are assigned to the cases of track splitting. They define whether track to split was selected among
  // mother candidates (3), daughter candidates (4), or tracks not passing any of these two criteria (5)
  short filterFlagToStore = 0;
  switch (filterFlag) {
    case 1:
    case 2:
    case 4:
    case 5:
      filterFlagToStore = 1;
      break;
    case 3:
    case 6:
      filterFlagToStore = 2;
      break;
    case 7:
      filterFlagToStore = 3;
      break;
    case 8:
      filterFlagToStore = 4;
      break;
    case 9:
      filterFlagToStore = 5;
  }

  // test the distance cut and remove pairs that do not pass the criteria
  // for split tracks, we do not remove the candidates, but fill the second digit of the flag to store
  if (distanceAtVertex > m_vertexDistanceCut) {
    if (filterFlag < 7)
      return false;
    else
      filterFlagToStore += 10;
  }

  // check if the fitted vertex is inside CDC or just after SVD
  if (vertexPos.Rho() < 14)
    return false;

  // extrapolate the mother state to IP (B2Vector3D(0., 0., 0.) beam spot, and B2Vector3D(0., 0., 1.) beam axis)
  genfit::MeasuredStateOnPlane stMotherIP = recoTrackMother->getMeasuredStateOnPlaneFromFirstHit();
  try {
    stMotherIP.extrapolateToLine(B2Vector3D(0., 0., 0.), B2Vector3D(0., 0., 1.));
  } catch (...) {
    B2DEBUG(29, "Could not extrapolate mother track to IP.");
  }

  // magnetic field at the fitted vertex and IP
  const double BzVtx = BFieldManager::getFieldInTesla(vertexPos).Z();
  const double BzIP = BFieldManager::getFieldInTesla({0, 0, 0}).Z();

  // prepare TrackFitResults for mother at IP and fitted vertex and for daughter at fitted vertex
  TrackFitResult* tfrMotherIP = buildTrackFitResult(m_motherKinkRecoTrackCache, stMotherIP, BzIP, Const::pion);
  TrackFitResult* tfrMotherVtx = buildTrackFitResult(m_motherKinkRecoTrackCache, stMother, BzVtx, Const::pion);
  TrackFitResult* tfrDaughterVtx = buildTrackFitResult(m_daughterKinkRecoTrackCache, stDaughter, BzVtx, Const::pion);

  // Try to combine tracks and fit them to find clones (excluding split tracks).
  // The result is written in the second and third digits of filter flag.
  if (m_kinkFitterModeCombineAndFit && (filterFlag < 7)) {
    unsigned int combinedFitFlag = combineTracksAndFit(trackMother, trackDaughter);
    filterFlagToStore += combinedFitFlag * 10;
  }

  // write to the filter flag number of reassigned hits (minus for daughter to mother, plus vice-versa)
  // since the type of flag to store is short, we are limited by +-32768.
  // The number of reassigned hits is rarely exceeds 32, so for that cases we feel the corresponding digits
  // with maximum available value of 32
  if (abs(finalHitPositionForReassignment) < 32) {
    if (finalHitPositionForReassignment >= 0)
      filterFlagToStore += finalHitPositionForReassignment * 1000;
    else {
      filterFlagToStore *= -1;
      filterFlagToStore += finalHitPositionForReassignment * 1000;
    }
  } else {
    filterFlagToStore += 32 * 1000;
  }

  // save the kink to the StoreArray
  m_kinks.appendNew(std::make_pair(trackMother, std::make_pair(tfrMotherIP, tfrMotherVtx)),
                    std::make_pair(trackDaughter, tfrDaughterVtx),
                    vertexPos.X(), vertexPos.Y(), vertexPos.Z(), filterFlagToStore);


  return true;
}

/// Fit kink vertex using RecoTracks as inputs.
/// Calculates distance at the fitted vertex.
/// Checks if the reassignment of the hits is required.
bool KinkFitter::vertexFitWithRecoTracks(RecoTrack* recoTrackMother, RecoTrack* recoTrackDaughter,
                                         unsigned int& reassignHitStatus,
                                         ROOT::Math::XYZVector& vertexPos, double& distance,
                                         ROOT::Math::XYZVector vertexPosSeed)
{

  // make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  genfit::AbsTrackRep* motherRepresentation = recoTrackMother->getCardinalRepresentation();
  if ((motherRepresentation == nullptr) || !(recoTrackMother->wasFitSuccessful(motherRepresentation))) {
    B2ERROR("Cardinal representation is not available for track. Should never happen, but I can continue safely anyway.");
    return false;
  }

  const double motherMass = TDatabasePDG::Instance()->GetParticle(motherRepresentation->getPDG())->Mass();
  const double motherCharge = recoTrackMother->getTrackFitStatus()->getCharge();

  // make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  genfit::AbsTrackRep* daughterRepresentation = recoTrackDaughter->getCardinalRepresentation();
  if ((daughterRepresentation == nullptr) || !(recoTrackDaughter->wasFitSuccessful(daughterRepresentation))) {
    B2ERROR("Cardinal representation is not available for track. Should never happen, but I can continue safely anyway.");
    return false;
  }

  const double daughterMass = TDatabasePDG::Instance()->GetParticle(daughterRepresentation->getPDG())->Mass();
  const double daughterCharge = recoTrackDaughter->getTrackFitStatus()->getCharge();

  // make a clone, not use the reference so that the genfit::MeasuredStateOnPlane and its TrackReps will not be altered.
  genfit::MeasuredStateOnPlane stMother = recoTrackMother->getMeasuredStateOnPlaneFromLastHit(motherRepresentation);
  genfit::MeasuredStateOnPlane stDaughter = recoTrackDaughter->getMeasuredStateOnPlaneClosestTo(vertexPosSeed,
                                            daughterRepresentation);
  m_stMotherBuffer = stMother;
  m_stDaughterBuffer = stDaughter;

  // initialize KVertexFitter
  analysis::VertexFitKFit kvf;

  // set magnetic field at the seed position
  const double Bz = BFieldManager::getFieldInTesla(vertexPosSeed).Z();
  kvf.setMagneticField(Bz);

  // set seed position
  HepPoint3D vertexPosSeedHepPoint(vertexPosSeed.X(), vertexPosSeed.Y(), vertexPosSeed.Z());
  kvf.setInitialVertex(vertexPosSeedHepPoint);

  // add mother state for the fit
  TVector3 motherPosition, motherMomentum;
  TMatrixDSym motherCovMatrix6(6, 6);
  stMother.getPosMomCov(motherPosition, motherMomentum, motherCovMatrix6);

  const double motherEnergy = sqrt(motherMomentum.Mag2() + motherMass * motherMass);
  ROOT::Math::PxPyPzEVector motherFourMomentum(motherMomentum.X(), motherMomentum.Y(),
                                               motherMomentum.Z(), motherEnergy);

  TMatrixDSym motherErrMatrixForKFit(7);
  errMatrixForKFit(motherFourMomentum, motherCovMatrix6, motherErrMatrixForKFit);

  kvf.addTrack(ROOTToCLHEP::getHepLorentzVector(motherFourMomentum),
               ROOTToCLHEP::getPoint3D(ROOT::Math::XYZVector(motherPosition)),
               ROOTToCLHEP::getHepSymMatrix(motherErrMatrixForKFit),
               motherCharge);

  // add daughter state for the fit
  TVector3 daughterPosition, daughterMomentum;
  TMatrixDSym daughterCovMatrix6(6, 6);
  stDaughter.getPosMomCov(daughterPosition, daughterMomentum, daughterCovMatrix6);

  const double daughterEnergy = sqrt(daughterMomentum.Mag2() + daughterMass * daughterMass);
  ROOT::Math::PxPyPzEVector daughterFourMomentum(daughterMomentum.X(), daughterMomentum.Y(),
                                                 daughterMomentum.Z(), daughterEnergy);

  TMatrixDSym daughterErrMatrixForKFit(7);
  errMatrixForKFit(daughterFourMomentum, daughterCovMatrix6, daughterErrMatrixForKFit);

  kvf.addTrack(ROOTToCLHEP::getHepLorentzVector(daughterFourMomentum),
               ROOTToCLHEP::getPoint3D(ROOT::Math::XYZVector(daughterPosition)),
               ROOTToCLHEP::getHepSymMatrix(daughterErrMatrixForKFit),
               daughterCharge);

  // do the fit
  int err = kvf.doFit();
  if (err != 0) {
    B2DEBUG(29, "Vertex fit finished with error");
    return false;
  }

  // test the chi2 cut
  if (kvf.getCHIsq() > m_vertexChi2Cut) {
    B2DEBUG(29, "Chi^2  of vertex fit is too large: " << kvf.getCHIsq());
    return false;
  }

  // get the fitted vertex
  HepPoint3D vertexPosHepPoint = kvf.getVertex();
  vertexPos = ROOT::Math::XYZVector(vertexPosHepPoint.x(), vertexPosHepPoint.y(), vertexPosHepPoint.z());

  // extrapolate the mother and the daughter states to the fitted vertex and get the status of the hit reassignment
  if (!extrapolateToVertex(stMother, stDaughter, vertexPos, reassignHitStatus)) {
    B2DEBUG(29, "Failed to extrapolate one of the tracks to the fitted kink vertex");
    return false;
  }

  // prepare mother Helix at the fitted vertex to calculate the distance between tracks
  const ROOT::Math::XYZVector motherPos = ROOT::Math::XYZVector(stMother.getPos());
  const ROOT::Math::XYZVector motherMom = ROOT::Math::XYZVector(stMother.getMom());
  Helix motherHelix(motherPos,
                    motherMom,
                    static_cast<short>(recoTrackMother->getTrackFitStatus()->getCharge()),
                    Bz);
  motherHelix.passiveMoveBy(vertexPos);

  // prepare daughter Helix at the fitted vertex to calculate the distance between tracks
  const ROOT::Math::XYZVector daughterPos = ROOT::Math::XYZVector(stDaughter.getPos());
  const ROOT::Math::XYZVector daughterMom = ROOT::Math::XYZVector(stDaughter.getMom());
  Helix daughterHelix(daughterPos,
                      daughterMom,
                      static_cast<short>(recoTrackDaughter->getTrackFitStatus()->getCharge()),
                      Bz);
  daughterHelix.passiveMoveBy(vertexPos);

  // calculate the distance between tracks as squared sum of their impact parameters at the fitted vertex
  distance = sqrt(daughterHelix.getD0() * daughterHelix.getD0() + daughterHelix.getZ0() * daughterHelix.getZ0() +
                  motherHelix.getD0() * motherHelix.getD0() + motherHelix.getZ0() * motherHelix.getZ0());

  // save mother and daughter states to the buffer
  m_stMotherBuffer = stMother;
  m_stDaughterBuffer = stDaughter;

  B2DEBUG(29, "Vertex fit with chi2 " << kvf.getCHIsq() << " and distance between tracks " << distance);
  return true;
}

/// Prepare the error matrix for the kFit
void KinkFitter::errMatrixForKFit(ROOT::Math::PxPyPzEVector& fourMomentum, TMatrixDSym& covMatrix6,
                                  TMatrixDSym& errMatrix7)
{

  enum {
    c_Px, c_Py, c_Pz, c_E, c_X, c_Y, c_Z
  };
  constexpr unsigned order[] = {c_X, c_Y, c_Z, c_Px, c_Py, c_Pz};

  for (int i = 0; i < 6; i++) {
    for (int j = i; j < 6; j++) {
      errMatrix7(order[j], order[i]) = errMatrix7(order[i], order[j]) = covMatrix6(i, j);
    }
  }

  const double dEdp[] = {fourMomentum.X() / fourMomentum.E(),
                         fourMomentum.Y() / fourMomentum.E(),
                         fourMomentum.Z() / fourMomentum.E()
                        };
  constexpr unsigned componentMom[] = {c_Px, c_Py, c_Pz};
  constexpr unsigned componentPos[] = {c_X, c_Y, c_Z};


  // covariances (p,E)
  for (unsigned int comp : componentMom) {
    double covariance = 0;
    for (int k = 0; k < 3; k++) {
      covariance += errMatrix7(comp, componentMom[k]) * dEdp[k];
    }
    errMatrix7(comp, c_E) = covariance;
  }

  // covariances (x,E)
  for (unsigned int comp : componentPos) {
    double covariance = 0;
    for (int k = 0; k < 3; k++) {
      covariance += errMatrix7(comp, componentMom[k]) * dEdp[k];
    }
    errMatrix7(c_E, comp) = covariance;
  }

  // variance (E,E)
  double covariance = 0;
  for (int i = 0; i < 3; i++) {
    covariance += errMatrix7(componentMom[i], componentMom[i]) * dEdp[i] * dEdp[i];
  }
  for (int i = 0; i < 3; i++) {
    int k = (i + 1) % 3;
    covariance += 2 * errMatrix7(componentMom[i], componentMom[k]) * dEdp[i] * dEdp[k];
  }
  errMatrix7(c_E, c_E) = covariance;

}
