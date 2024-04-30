/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/kinkFinding/fitter/kinkFitter.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/geometry/VectorUtil.h>
#include <framework/geometry/BFieldManager.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/trackFitting/trackBuilder/factories/TrackBuilder.h>
#include <tracking/kinkFinding/fitter/kinkVertexFitterKFit.h>

#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>

#include <genfit/Track.h>
#include <genfit/TrackPoint.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/KalmanFitterRefTrack.h>

#include <CLHEP/Matrix/Matrix.h>
#include <CLHEP/Matrix/SymMatrix.h>
#include <CLHEP/Vector/LorentzVector.h>
#include <CLHEP/Geometry/Point3D.h>

#include <framework/utilities/IOIntercept.h>

using namespace Belle2;

kinkFitter::kinkFitter(const std::string& trackFitResultsName, const std::string& kinksName,
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

  m_motherKinkRecoTracks.registerInDataStore("motherKinkRecoTracks", DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_motherKinkRecoTracks);
  m_motherKinkRecoTracks.registerRelationTo(m_recoTracks);

  m_daughterKinkRecoTracks.registerInDataStore("daughterKinkRecoTracks", DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_daughterKinkRecoTracks);
  m_daughterKinkRecoTracks.registerRelationTo(m_recoTracks);
  m_daughterKinkRecoTracks.registerRelationTo(m_copiedRecoTracks);


  B2ASSERT("Material effects not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::MaterialEffects::getInstance()->isInitialized());
  B2ASSERT("Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::FieldManager::getInstance()->isInitialized());
}

void kinkFitter::setFitterMode(int fitterMode)
{
  if (not(0 <= fitterMode && fitterMode <= 7)) {
    B2FATAL("Invalid fitter mode!");
  } else {
    m_kinkFitterMode = fitterMode;
    // filling bits of the fitter mode
    m_kinkFitterModeHitsReassignment = fitterMode & 1;
    fitterMode >>= 1;
    m_kinkFitterModeFlipAndRefit = fitterMode & 1;
    fitterMode >>= 1;
    m_kinkFitterModeCombineAndFit = fitterMode & 1;
  }
}

void kinkFitter::initializeCuts(double vertexDistanceCut,
                                double vertexChi2Cut)
{
  m_vertexDistanceCut = vertexDistanceCut;
  m_vertexChi2Cut = vertexChi2Cut;
}

/// Used in the vertexFitWithRecoTracks function to extrapolate the states to the fitted vertex.
/// If the vertex is inside one of the tracks, bits in hasInnerHitStatus are set.
bool kinkFitter::extrapolateToVertex(genfit::MeasuredStateOnPlane& stMother, genfit::MeasuredStateOnPlane& stDaughter,
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
TrackFitResult* kinkFitter::buildTrackFitResult(RecoTrack* recoTrack,
                                                const genfit::MeasuredStateOnPlane& msop,
                                                const double Bz,
                                                const Const::ParticleType trackHypothesis)
{
  const uint64_t hitPatternCDCInitializer = TrackBuilder::getHitPatternCDCInitializer(*recoTrack);
  uint32_t hitPatternVXDInitializer = TrackBuilder::getHitPatternVXDInitializer(*recoTrack);
  genfit::Track track = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack);

  TrackFitResult* kinkTrackFitResult
    = m_trackFitResults.appendNew(ROOT::Math::XYZVector(msop.getPos()), ROOT::Math::XYZVector(msop.getMom()),
                                  msop.get6DCov(), msop.getCharge(),
                                  trackHypothesis,
                                  track.getFitStatus()->getPVal(),
                                  Bz, hitPatternCDCInitializer, hitPatternVXDInitializer,
                                  track.getFitStatus()->getNdf());
  return kinkTrackFitResult;
}

/// Find hit position closest to the vertex.
int kinkFitter::findHitPositionForReassignment(RecoTrack* recoTrack,
                                               ROOT::Math::XYZVector& vertexPos,
                                               int direction)
{

  // Helper variables to store the minimum
  double minimalDistance2 = 0;
  int minimalIndex = 0;

  // CDC Hits list to loop over
  auto cdcHits = recoTrack->getSortedCDCHitList();

  if (direction > 0) {
    for (int ri = 0; ri < static_cast<int>(cdcHits.size()); ++ri) {
      auto recoHitInfo = recoTrack->getRecoHitInformation(cdcHits[ri]);
      if (!recoHitInfo->useInFit()) continue;
      try {
        const genfit::MeasuredStateOnPlane& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromRecoHit(recoHitInfo);
        const double currentDistance2 = (ROOT::Math::XYZVector(measuredStateOnPlane.getPos()) - vertexPos).Mag2();

        if (not minimalDistance2 or currentDistance2 < minimalDistance2) {
          minimalDistance2 = currentDistance2;
          minimalIndex = ri;
        }
        // if cannot find minimum in 3 iterations, stop searching
        if (ri - minimalIndex > 3) break;
      } catch (const NoTrackFitResult& exception) {
        B2DEBUG(50, "Can not get mSoP because of: " << exception.what());
        continue;
      } catch (const genfit::Exception& exception) {
        B2DEBUG(50, "Can not get mSoP because of: " << exception.what());
        continue;
      }
    }
    if (minimalIndex == 0)
      return -1;
    else
      return -(minimalIndex);
  } else {
    for (int ri = cdcHits.size()  - 1; ri >= 0; --ri) {
      auto recoHitInfo = recoTrack->getRecoHitInformation(cdcHits[ri]);
      if (!recoHitInfo->useInFit()) continue;
      try {
        const genfit::MeasuredStateOnPlane& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromRecoHit(recoHitInfo);
        const double currentDistance2 = (ROOT::Math::XYZVector(measuredStateOnPlane.getPos()) - vertexPos).Mag2();

        if (not minimalDistance2 or currentDistance2 < minimalDistance2) {
          minimalDistance2 = currentDistance2;
          minimalIndex = ri;
        }
        // if cannot find minimum in 3 iterations, stop searching
        if (minimalIndex - ri > 3) break;
      } catch (const NoTrackFitResult& exception) {
        B2DEBUG(50, "Can not get mSoP because of: " << exception.what());
        continue;
      } catch (const genfit::Exception& exception) {
        B2DEBUG(50, "Can not get mSoP because of: " << exception.what());
        continue;
      }
    }
    if ((cdcHits.size() - minimalIndex) == 0)
      return 1;
    else
      return (cdcHits.size() - minimalIndex);
  }
}

/// Copy RecoTrack to a separate StoreArray and reassign CDC hits according to delta
RecoTrack* kinkFitter::copyRecoTrackAndReassignCDCHits(RecoTrack* motherRecoTrack, RecoTrack* daughterRecoTrack,
                                                       bool motherFlag, int delta)
{

  // lists of CDC hits of mother and daughter tracks
  auto motherCDCHit = motherRecoTrack->getSortedCDCHitList();
  auto daughterCDCHit = daughterRecoTrack->getSortedCDCHitList();

  // initialization of helper variables
  int deltaMother = 0;
  int deltaDaughter = 0;
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
                                                      static_cast<short>(recoTrackToCopy->getChargeSeed()));
  recoTrack->setTimeSeed(recoTrackToCopy->getTimeSeed());
  recoTrack->setSeedCovariance(recoTrackToCopy->getSeedCovariance());

  // copy PXD hits
  for (auto* pxdHit : recoTrackToCopy->getPXDHitList()) {
    auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(pxdHit);
    recoTrack->addPXDHit(pxdHit, recoHitInfo->getSortingParameter(),
                         recoHitInfo->getFoundByTrackFinder());
  }

  // copy SVD hits
  for (auto* svdHit : recoTrackToCopy->getSVDHitList()) {
    auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(svdHit);
    recoTrack->addSVDHit(svdHit, recoHitInfo->getSortingParameter(),
                         recoHitInfo->getFoundByTrackFinder());
  }

  // sorting parameter after copying VXD hits
  unsigned int sortingParameterVXDOffset = recoTrack->getNumberOfTotalHits();

  // copy CDC hits with respect to reassignment
  if (motherFlag) {
    if (delta > 0) {
      deltaMother = -delta;
    } else {
      deltaDaughter = -delta;
    }
    for (size_t mi = 0; mi < motherCDCHit.size() + deltaMother; ++mi) {
      auto recoHitInfo = motherRecoTrack->getRecoHitInformation(motherCDCHit[mi]);
      recoTrack->addCDCHit(motherCDCHit[mi],
                           recoHitInfo->getSortingParameter(),
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }
    sortingParameterOffset = recoTrack->getNumberOfTotalHits();
    for (size_t di = 0; di < static_cast<unsigned int>(deltaDaughter); ++di) {
      auto recoHitInfo = daughterRecoTrack->getRecoHitInformation(daughterCDCHit[di]);
      /*if (!recoHitInfo->useInFit()) recoHitInfo->setUseInFit(true);*/
      recoTrack->addCDCHit(daughterCDCHit[di],
                           recoHitInfo->getSortingParameter() + sortingParameterOffset,
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }
    sortingParameterOffset = deltaDaughter + deltaMother;
  } else {
    if (delta > 0) {
      deltaMother = -delta;
    } else {
      deltaDaughter = -delta;
    }
    if (deltaMother)
      sortingParameterOffset = motherRecoTrack->getRecoHitInformation(motherCDCHit[motherCDCHit.size() +
                                                   deltaMother])->getSortingParameter();
    for (size_t mi = motherCDCHit.size() + deltaMother; mi < motherCDCHit.size(); ++mi) {
      auto recoHitInfo = motherRecoTrack->getRecoHitInformation(motherCDCHit[mi]);
      recoTrack->addCDCHit(motherCDCHit[mi],
                           sortingParameterVXDOffset + recoHitInfo->getSortingParameter() - sortingParameterOffset,
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }
    sortingParameterOffset = -deltaMother - deltaDaughter;
    for (size_t di = deltaDaughter; di < daughterCDCHit.size(); ++di) {
      auto recoHitInfo = daughterRecoTrack->getRecoHitInformation(daughterCDCHit[di]);
      recoTrack->addCDCHit(daughterCDCHit[di],
                           recoHitInfo->getSortingParameter() + sortingParameterOffset,
                           recoHitInfo->getRightLeftInformation(),
                           recoHitInfo->getFoundByTrackFinder());

    }
  }

  // copy BKLM hits
  for (auto* bklmHit : recoTrackToCopy->getBKLMHitList()) {
    auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(bklmHit);
    recoTrack->addBKLMHit(bklmHit, recoHitInfo->getSortingParameter() + sortingParameterOffset,
                          recoHitInfo->getFoundByTrackFinder());
  }

  // copy EKLM hits
  for (auto* eklmHit : recoTrackToCopy->getEKLMHitList()) {
    auto recoHitInfo = recoTrackToCopy->getRecoHitInformation(eklmHit);
    recoTrack->addEKLMHit(eklmHit, recoHitInfo->getSortingParameter() + sortingParameterOffset,
                          recoHitInfo->getFoundByTrackFinder());
  }

  return recoTrack;

}

/// Try to fit new RecoTracks after hit reassignment.
bool kinkFitter::refitRecoTrackAfterReassign(RecoTrack* recoTrackMotherRefit, RecoTrack* recoTrackDaughterRefit,
                                             RecoTrack* recoTrackMother, RecoTrack* recoTrackDaughter)
{

  // fitter initialization
  TrackFitter trackFitter;

  const genfit::FitStatus* motherTrackFitStatus = recoTrackMother->getTrackFitStatus();
  const genfit::FitStatus* daughterTrackFitStatus = recoTrackDaughter->getTrackFitStatus();

  B2DEBUG(29, "Initial mother fit result " << motherTrackFitStatus->getPVal() << " " << motherTrackFitStatus->getNdf());
  B2DEBUG(29, "Initial daughter fit result " << daughterTrackFitStatus->getPVal() << " "
          << daughterTrackFitStatus->getNdf());

  // fit the new tracks
  trackFitter.fit(*recoTrackMotherRefit);
  trackFitter.fit(*recoTrackDaughterRefit);

  if (!recoTrackMotherRefit->wasFitSuccessful() || !recoTrackDaughterRefit->wasFitSuccessful()) {
    B2DEBUG(29, "Refit of the tracks with reassigned hits failed ");
    return false;
  }

  const genfit::FitStatus* motherNewTrackFitStatus = recoTrackMotherRefit->getTrackFitStatus();
  const genfit::FitStatus* daughterNewTrackFitStatus = recoTrackDaughterRefit->getTrackFitStatus();

  B2DEBUG(29, "New mother fit result " << motherNewTrackFitStatus->getPVal() << " " << motherNewTrackFitStatus->getNdf());
  B2DEBUG(29, "New daughter fit result " << daughterNewTrackFitStatus->getPVal() << " "
          << daughterNewTrackFitStatus->getNdf());

  // Fit is assumed to be successful if it improves PValue of one of the tracks, while not decreasing the second track
  // Pvalue much. If the daughter was fitted badly initially, ignore its PValue.
  if ((motherNewTrackFitStatus->getPVal() > motherTrackFitStatus->getPVal() &&
       (daughterNewTrackFitStatus->getPVal() / daughterTrackFitStatus->getPVal() > 0.5 ||
        daughterTrackFitStatus->getPVal() < 0.0001)) ||
      (daughterNewTrackFitStatus->getPVal() > daughterTrackFitStatus->getPVal() &&
       motherNewTrackFitStatus->getPVal() / motherTrackFitStatus->getPVal() > 0.5))
    return true;
  else
    return false;
}

/// Flip and refit the daughter track.
RecoTrack* kinkFitter::copyRecoTrackForFlipAndRefit(RecoTrack* recoTrack,
                                                    ROOT::Math::XYZVector& momentumSeed,
                                                    ROOT::Math::XYZVector& positionSeed,
                                                    double& timeSeed)
{

  // fitter initialization
  TrackFitter trackFitter;

  // DAF fitter usually works badly with flipped tracks, so kalmanFitter is used instead
  std::shared_ptr<genfit::KalmanFitterRefTrack> kalmanFitter = std::make_shared<genfit::KalmanFitterRefTrack>();
  kalmanFitter->setMinIterations(static_cast<unsigned int>(3));
  kalmanFitter->setMaxIterations(static_cast<unsigned int>(10));
  kalmanFitter->setMaxFailedHits(static_cast<unsigned int>(5));

  trackFitter.resetFitter(kalmanFitter);

  // copy recoTracks to a separate StoreArray using seed information
  RecoTrack* newRecoTrack = m_copiedRecoTracks.appendNew(positionSeed,
                                                         -momentumSeed,
                                                         static_cast<short>(-recoTrack->getChargeSeed()));
  newRecoTrack->setTimeSeed(timeSeed);
  newRecoTrack->setSeedCovariance(recoTrack->getSeedCovariance());
  newRecoTrack->addHitsFromRecoTrack(recoTrack, newRecoTrack->getNumberOfTotalHits(), true);

  // fit the new track
  trackFitter.fit(*newRecoTrack);

  return newRecoTrack;
}

/// Refit the daughter track blocking hits if required.
RecoTrack* kinkFitter::copyRecoTrackForRefit(RecoTrack* recoTrack,
                                             ROOT::Math::XYZVector& momentumSeed,
                                             ROOT::Math::XYZVector positionSeed,
                                             double& timeSeed,
                                             bool block = false, bool useAnotherFitter = false)
{

  // fitter initialization
  TrackFitter trackFitter;

  // if true, set ordinary KalmanFilter (for filter 6 it performs better than DAF)
  if (useAnotherFitter) {
    std::shared_ptr<genfit::KalmanFitterRefTrack> kalmanFitter = std::make_shared<genfit::KalmanFitterRefTrack>();
    kalmanFitter->setMinIterations(static_cast<unsigned int>(3));
    kalmanFitter->setMaxIterations(static_cast<unsigned int>(10));
    kalmanFitter->setMaxFailedHits(static_cast<unsigned int>(5));

    trackFitter.resetFitter(kalmanFitter);
  }

  // copy recoTracks to a separate StoreArray using seed information
  RecoTrack* newRecoTrack = m_copiedRecoTracks.appendNew(positionSeed,
                                                         momentumSeed,
                                                         static_cast<short>(recoTrack->getChargeSeed()));
  newRecoTrack->setTimeSeed(timeSeed);
  newRecoTrack->setSeedCovariance(recoTrack->getSeedCovariance());
  newRecoTrack->addHitsFromRecoTrack(recoTrack, newRecoTrack->getNumberOfTotalHits());

  // block the hits in the first stereo layer and all before (leave at least 6 hits for fit)
  // (usually, wrong assignment of first stereo layer is responsible for wrong z coordinate)
  if (block) {
    bool passedStereo = false;
    auto newCDCHitRefit = newRecoTrack->getSortedCDCHitList();
    int di = 0;
    for (; di < static_cast<int>(newCDCHitRefit.size()) - 6; ++di) {
      if (!passedStereo && (newCDCHitRefit[di]->getISuperLayer() % 2 != 0))
        passedStereo = true;
      if (passedStereo && (newCDCHitRefit[di]->getISuperLayer() % 2 == 0))
        break;
      auto recoHitInfo = newRecoTrack->getRecoHitInformation(newCDCHitRefit[di]);
      recoHitInfo->setUseInFit(false);
    }
    B2DEBUG(29, "Blocked hits for refit: " << di);
  }

  // fit the new track
  trackFitter.fit(*newRecoTrack);

  return newRecoTrack;
}

/// check if the refit of filter 6 daughter tracks improves the distance between mother and daughter
bool kinkFitter::isRefitImproveFilter6(RecoTrack* recoTrackDaughterRefit, TVector3& motherPosLast)
{
  // get the values near the mother last point
  TVector3 daughterPosClosestToMotherPosLast = recoTrackDaughterRefit->getMeasuredStateOnPlaneFromFirstHit().getPos();
  TVector3 daughterMomClosestToMotherPosLast = recoTrackDaughterRefit->getMeasuredStateOnPlaneFromFirstHit().getMom();
  const double Bz = BFieldManager::getFieldInTesla({daughterPosClosestToMotherPosLast.X(),
                                                    daughterPosClosestToMotherPosLast.Y(),
                                                    daughterPosClosestToMotherPosLast.Z()}).Z();
  // daughter Helix with move to mother last point
  Helix daughterHelixClosestToMotherPosLast(ROOT::Math::XYZVector(daughterPosClosestToMotherPosLast),
                                            ROOT::Math::XYZVector(daughterMomClosestToMotherPosLast),
                                            static_cast<short>(recoTrackDaughterRefit->getTrackFitStatus()->getCharge()),
                                            Bz);
  daughterHelixClosestToMotherPosLast.passiveMoveBy(ROOT::Math::XYZVector(motherPosLast));

  // check if the 3D distance passes loose criteria (in default fit this test is failed)
  return (sqrt(daughterHelixClosestToMotherPosLast.getD0() * daughterHelixClosestToMotherPosLast.getD0() +
               daughterHelixClosestToMotherPosLast.getZ0() * daughterHelixClosestToMotherPosLast.getZ0()) < 10);
}

/// combine daughter and mother tracks and fit the result to check for clones
int kinkFitter::combineTracksAndFit(const Track* trackMother, const Track* trackDaughter)
{
  RecoTrack* recoTrackMother = trackMother->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackDaughter = trackDaughter->getRelated<RecoTrack>(m_recoTracksName);

  // create a combined track by reassigning all daughter track hits to mother track
  RecoTrack* recoTrackCombinedRefit = copyRecoTrackAndReassignCDCHits(recoTrackMother,
                                      recoTrackDaughter, true, -recoTrackDaughter->getNumberOfCDCHits());

  // fitter initialization
  TrackFitter trackFitter;
  // fit the new track
  trackFitter.fit(*recoTrackCombinedRefit);

  // return 99 if the track fit failed
  if (!recoTrackCombinedRefit->wasFitSuccessful()) {
    B2DEBUG(29, "Refit of the combined track failed ");
    return 19;
  }

  // fit results of daughter, mother, and new combined tracks
  const genfit::FitStatus* motherTrackFitStatus = recoTrackMother->getTrackFitStatus();
  const genfit::FitStatus* daughterTrackFitStatus = recoTrackDaughter->getTrackFitStatus();
  const genfit::FitStatus* combinedTrackFitStatus = recoTrackCombinedRefit->getTrackFitStatus();

  B2DEBUG(29, "Initial mother fit result " << motherTrackFitStatus->getPVal() << " " << motherTrackFitStatus->getNdf());
  B2DEBUG(29, "Initial daughter fit result " << daughterTrackFitStatus->getPVal() << " "
          << daughterTrackFitStatus->getNdf());
  B2DEBUG(29, "Combined track fit result " << combinedTrackFitStatus->getPVal() << " " << combinedTrackFitStatus->getNdf());

  // return 98 if the combined track has NDF less than mother track
  if (combinedTrackFitStatus->getNdf() < motherTrackFitStatus->getNdf())
    return 18;

  // filling bits according to the fit result of the combined track
  int motherFlag = (combinedTrackFitStatus->getPVal() > motherTrackFitStatus->getPVal());
  int daughterFlag = 2 * (combinedTrackFitStatus->getPVal() > daughterTrackFitStatus->getPVal());
  int daughterNdfFlag = 4 * (combinedTrackFitStatus->getNdf() > daughterTrackFitStatus->getNdf());
  int pValueFlag = 8 * (combinedTrackFitStatus->getPVal() > 0.0000001);

  return motherFlag + daughterFlag + daughterNdfFlag + pValueFlag;
}

/// Fit and store kink.
/// If the fitterMode requires, tries to reassign hits between daughter and mother tracks.
/// If the fitterMode requires, tries to flip and refit the tracks.
bool kinkFitter::fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag)
{

  // Existence of corresponding RecoTrack already checked at the module level;
  RecoTrack* recoTrackMother = trackMother->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackDaughter = trackDaughter->getRelated<RecoTrack>(m_recoTracksName);

  // temporary StoreArrays for studies (to be removed)
  RecoTrack* motherKinkRecoTrack = recoTrackMother->copyToStoreArray(m_motherKinkRecoTracks);
  motherKinkRecoTrack->addHitsFromRecoTrack(recoTrackMother, motherKinkRecoTrack->getNumberOfTotalHits());
  motherKinkRecoTrack->addRelationTo(recoTrackMother);

  RecoTrack* daughterKinkRecoTrack = recoTrackDaughter->copyToStoreArray(m_daughterKinkRecoTracks);
  daughterKinkRecoTrack->addHitsFromRecoTrack(recoTrackDaughter, daughterKinkRecoTrack->getNumberOfTotalHits());
  daughterKinkRecoTrack->addRelationTo(recoTrackDaughter);

  // Tracks selected with filter from 4 to 6 are selected by 2D distance cut assuming bad z coordinate.
  // Refit is required for such tracks in the majority of the cases.
  // If the refit successful, use new RecoTrack for the vertex fit.

  // Refit filteFlag 4 and 6, which do not require flipping.
  bool refitBadFlag = false;
  if (filterFlag == 4 || filterFlag == 6) {
    B2DEBUG(29, "Try to refit filter " << filterFlag);

    // initialize seeds for the refit
    // position of the last mother state and the first daughter state
    TVector3 motherPosLast = recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos();
    TVector3 daughterPosFirst = recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getPos();
    // use mother last state time as a seed for the daughter track
    double timeSeedDaughterRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                     recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the first hit for a momentum seed
    ROOT::Math::XYZVector momSeedDaughterRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getMom());

    // initialize refit conditions
    // remove hits until the first stereo layer is passed
    bool blockHits = false;
    if (filterFlag == 4) blockHits = true;
    // use ordinary KalmanFilter
    bool anotherFitter = false;
    if (filterFlag == 6) anotherFitter = true;

    // create a copy of the daughter track and refit it
    RecoTrack* recoTrackDaughterRefit = copyRecoTrackForRefit(recoTrackDaughter,
                                                              momSeedDaughterRefit,
                                                              ROOT::Math::XYZVector(motherPosLast),
                                                              timeSeedDaughterRefit, blockHits, anotherFitter);

    // if the new track fit is successful, and in addition, the distance for the filter 6 is improved,
    // use it for the vertex fit
    if (recoTrackDaughterRefit->wasFitSuccessful()) {
      if (filterFlag == 4 ||
          (filterFlag == 6 && isRefitImproveFilter6(recoTrackDaughterRefit, motherPosLast)))
        recoTrackDaughter = recoTrackDaughterRefit;
      B2DEBUG(29, "Refit successful");
      refitBadFlag = true;
    }
  }

  // Flip and refit filter 5.
  if ((m_kinkFitterMode == 2 || m_kinkFitterMode == 3) && (filterFlag == 5)) {
    B2DEBUG(29, "Try to flip and refit filter " << filterFlag);

    // initialize seeds for the refit
    // use position of the last mother state as a seed for the daughter track
    ROOT::Math::XYZVector motherPosLastXYZ = ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos());
    // use mother last state time as a seed for the daughter track
    double timeSeedDaughterFlipAndRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                            recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the last hit for a momentum seed
    ROOT::Math::XYZVector momSeedDaughterFlipAndRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromLastHit().getMom());

    // create a copy of the daughter track, flipped and refitted
    RecoTrack* recoTrackDaughterFlipAndRefit = copyRecoTrackForFlipAndRefit(recoTrackDaughter,
                                               motherPosLastXYZ, momSeedDaughterFlipAndRefit,
                                               timeSeedDaughterFlipAndRefit);

    // if the new track fit is successful, use it for the vertex fit
    if (recoTrackDaughterFlipAndRefit->wasFitSuccessful()) {
      recoTrackDaughter = recoTrackDaughterFlipAndRefit;
      B2DEBUG(29, "Flip and refit successful");
    }
  }

  // fitted vertex position
  ROOT::Math::XYZVector vertexPos = ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos());

  // flag to reassign hits, final hit to reassign, and distance at the fitted vertex
  unsigned int reassignHitStatus = 0;
  int finalHitPositionForReassignment = 0;
  double distanceAtVertex = 10000;

  // Try kink vertex fit. If the fit fails, return false immediately for all except filter 1 and 3.
  bool failedFitFlag = false;
  if (!vertexFitWithRecoTracks(recoTrackMother, recoTrackDaughter, reassignHitStatus, vertexPos, distanceAtVertex,
                               ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos())))
    failedFitFlag = true;

  if (failedFitFlag && (filterFlag != 1 && filterFlag != 3))
    return false;

  // If the fit fails for filter 1, try to refit daughter track blocking the first stereo superlayer.
  if (failedFitFlag && filterFlag == 1) {
    B2DEBUG(29, "Try to refit filter " << filterFlag);

    // initialize seeds for the refit
    // position of the last mother state and the first daughter state
    TVector3 motherPosLast = recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos();
    TVector3 daughterPosFirst = recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getPos();
    // use mother last state time as a seed for the daughter track
    double timeSeedDaughterRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                     recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the first hit for a momentum seed
    ROOT::Math::XYZVector momSeedDaughterRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getMom());

    // initialize refit conditions
    // remove hits until the first stereo layer is passed
    bool blockHits = true;
    // do not use ordinary KalmanFilter
    bool anotherFitter = false;

    // create a copy of the daughter track and refit it
    RecoTrack* recoTrackDaughterRefit = copyRecoTrackForRefit(recoTrackDaughter,
                                                              momSeedDaughterRefit,
                                                              ROOT::Math::XYZVector(motherPosLast),
                                                              timeSeedDaughterRefit, blockHits, anotherFitter);

    // if the new track fit is successful, and in addition, the vertex fit is successful,
    // use a new fit and proceed. Otherwise, return false.
    if (recoTrackDaughterRefit->wasFitSuccessful() &&
        vertexFitWithRecoTracks(recoTrackMother, recoTrackDaughterRefit, reassignHitStatus, vertexPos, distanceAtVertex,
                                ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos()))) {
      recoTrackDaughter = recoTrackDaughterRefit;
      B2DEBUG(29, "Refit successful");
      refitBadFlag = true;
    } else return false;
  }

  // If the filter 4 or 1 (if required) was refitted successfully, reassignHitStatus may not be assigned due to blocked hits.
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
  if (m_kinkFitterModeHitsReassignment && (filterFlag == 1 || filterFlag == 4) && (reassignHitStatus != 0)) {
    B2DEBUG(29, "Start of the hits reassignment for filter " << filterFlag);

    // initialize counter for reassigning tries
    unsigned int count_reassign_tries = 0;

    // variables to store temporary values
    int finalHitPositionForReassignmentTmp = 0;
    double distanceAtVertexTmp = 1000;
    ROOT::Math::XYZVector vertexPosTmp(vertexPos);
    RecoTrack* recoTrackMotherRefit = nullptr;
    RecoTrack* recoTrackDaughterRefit = nullptr;
    RecoTrack* recoTrackMotherBuffer = recoTrackMother;
    RecoTrack* recoTrackDaughterBuffer = recoTrackDaughter;

    // The number of tries is limited to 3
    while (reassignHitStatus != 0 && count_reassign_tries < 3) {
      ++count_reassign_tries;
      B2DEBUG(29, "Try number " << count_reassign_tries);

      // find threshold hit position in the track
      // hit bellow the threshold are to be reassigned
      // positive for daughter hits reassignment to mother, negative vice-versa
      int hitPositionForReassignment = 0;
      if (reassignHitStatus & 0x1) {

        // daughter hits to be reassigned
        hitPositionForReassignment = findHitPositionForReassignment(recoTrackDaughterBuffer, vertexPosTmp, 1);

        // test if the number of hits to be reassigned larger than the number of CDC hits in daughter tracks
        // minus number degree of freedom required for the fit
        if (static_cast<unsigned int>(-hitPositionForReassignment + 5) >
            recoTrackDaughterBuffer->getNumberOfCDCHits())
          break;

      } else if (reassignHitStatus & 0x2) {

        // mother hits to be reassigned
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
      B2DEBUG(29, "Found hit index to reassign " << hitPositionForReassignment);

      // refit of the new tracks can fail when the position is too far
      // try positions closer to the end until reach it
      while (abs(hitPositionForReassignment) > 0) {

        // create new RecoTracks with reassigned hits in the separate StoreArray
        recoTrackMotherRefit = copyRecoTrackAndReassignCDCHits(recoTrackMotherBuffer,
                                                               recoTrackDaughterBuffer,
                                                               true, hitPositionForReassignment);
        recoTrackDaughterRefit = copyRecoTrackAndReassignCDCHits(recoTrackMotherBuffer,
                                                                 recoTrackDaughterBuffer,
                                                                 false, hitPositionForReassignment);

        // try to fit new RecoTracks assuming improvement of the result
        // if fit fails, try position closer to the end
        // if fit is successful, break the loop
        if (!refitRecoTrackAfterReassign(recoTrackMotherRefit, recoTrackDaughterRefit,
                                         recoTrackMother, recoTrackDaughter)) {
          if (hitPositionForReassignment > 0)
            --hitPositionForReassignment;
          else
            ++hitPositionForReassignment;
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
      if (distanceAtVertexTmp < distanceAtVertex) {
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

  // if the corresponding fitterMode is used, try to flip the daughter track from filter 2 and refit it
  if (m_kinkFitterModeFlipAndRefit && (filterFlag == 2)) {
    B2DEBUG(29, "Try to flip and refit filter " << filterFlag);

    // variables to store temporary values
    double distanceAtVertexTmp = 1000;
    ROOT::Math::XYZVector vertexPosTmp(vertexPos);

    // use mother last state time as a seed for the daughter track
    double timeSeedDaughterFlipAndRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                            recoTrackMother->getMeasuredStateOnPlaneFromLastHit());

    // use state at the fitted vertex for a momentum seed
    ROOT::Math::XYZVector momSeedDaughterFlipAndRefit(stDaughter.getMom());

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

        B2DEBUG(29, "Flip and refit successful");
      }
  }

  // try to refit daughter track for filter 3. If it improves the vertex fit, which might even fail before,
  // use a new result
  if (filterFlag == 3) {
    B2DEBUG(29, "Try to refit filter " << filterFlag);

    // initialize seeds for the refit
    // position of the last mother state and the first daughter state
    TVector3 motherPosLast = recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos();
    // use mother last state time as a seed for the daughter track
    double timeSeedDaughterRefit = recoTrackMother->getCardinalRepresentation()->getTime(
                                     recoTrackMother->getMeasuredStateOnPlaneFromLastHit());
    // use fitted state at the first hit for a momentum seed
    ROOT::Math::XYZVector momSeedDaughterRefit(recoTrackDaughter->getMeasuredStateOnPlaneFromFirstHit().getMom());

    // variables to store temporary values
    double distanceAtVertexTmp = 1000;
    ROOT::Math::XYZVector vertexPosTmp(vertexPos);

    // initialize refit conditions
    // do not remove hits until the first stereo layer is passed
    bool blockHits = false;
    // use ordinary KalmanFilter
    bool anotherFitter = true;

    // create a copy of the daughter track and refit it
    RecoTrack* recoTrackDaughterRefit = copyRecoTrackForRefit(recoTrackDaughter,
                                                              momSeedDaughterRefit,
                                                              ROOT::Math::XYZVector(motherPosLast),
                                                              timeSeedDaughterRefit, blockHits, anotherFitter);

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

        B2DEBUG(29, "Refit successful");
      }
    } else if (failedFitFlag) return false;
  }

  B2DEBUG(29, "Distance between tracks at fitted vertex " << distanceAtVertex);
  B2DEBUG(29, "Radius of the vertex " << vertexPos.Rho());
  B2DEBUG(29, "Reassigned hit " << finalHitPositionForReassignment);

  // test the distance cut
  if (distanceAtVertex > m_vertexDistanceCut)
    return false;

  // check if the fitted vertex is inside CDC or just after SVD
  if (vertexPos.Rho() < 14)
    return false;

  // extrapolate the mother state to IP
  genfit::MeasuredStateOnPlane stMotherIP = recoTrackMother->getMeasuredStateOnPlaneFromFirstHit();
  try {
    stMotherIP.extrapolateToPoint(TVector3(0, 0, 0));
  } catch (...) {
    B2DEBUG(29, "Could not extrapolate mother track to IP.");
  }

  // magnetic field at the fitted vertex and IP
  const double BzVtx = BFieldManager::getFieldInTesla({vertexPos.X(), vertexPos.Y(), vertexPos.Z()}).Z();
  const double BzIP = BFieldManager::getFieldInTesla({0, 0, 0}).Z();

  // prepare TrackFitResults for mother at IP and fitted vertex and for daughter at fitted vertex
  TrackFitResult* tfrMotherIP = buildTrackFitResult(m_motherKinkRecoTrackCache, stMotherIP, BzIP, Const::pion);
  TrackFitResult* tfrMotherVtx = buildTrackFitResult(m_motherKinkRecoTrackCache, stMother, BzVtx, Const::pion);
  TrackFitResult* tfrDaughterVtx = buildTrackFitResult(m_daughterKinkRecoTrackCache, stDaughter, BzVtx, Const::pion);

  // write to the filter flag number of reassigned hits (minus for daughter to mother, plus vice-versa)
  if (abs(finalHitPositionForReassignment) < 32) {
    if (finalHitPositionForReassignment >= 0)
      filterFlag += finalHitPositionForReassignment * 1000;
    else {
      filterFlag *= -1;
      filterFlag += finalHitPositionForReassignment * 1000;
    }
  } else {
    filterFlag += 32 * 1000;
  }

  // try to combine tracks and fit them to find clones. The result is written to filter flag
  if (m_kinkFitterModeCombineAndFit) {
    int combinedFitFlag = combineTracksAndFit(trackMother, trackDaughter);
    if (filterFlag >= 0) {
      filterFlag += combinedFitFlag * 10;
    } else {
      filterFlag -= combinedFitFlag * 10;
    }
  }

  // save the kink to the StoreArray
  auto kink = m_kinks.appendNew(std::make_pair(trackMother, std::make_pair(tfrMotherIP, tfrMotherVtx)),
                                std::make_pair(trackDaughter, tfrDaughterVtx),
                                vertexPos.X(), vertexPos.Y(), vertexPos.Z(), filterFlag);


  return true;
}

/// Fit kink vertex using RecoTracks as inputs.
/// Calculates distance at the fitted vertex.
/// Checks if the reassignment of the hits is required.
bool kinkFitter::vertexFitWithRecoTracks(RecoTrack* recoTrackMother, RecoTrack* recoTrackDaughter,
                                         unsigned int& reassignHitStatus,
                                         ROOT::Math::XYZVector& vertexPos, double& distance,
                                         ROOT::Math::XYZVector vertexPosSeed = ROOT::Math::XYZVector(0, 0, 0))
{

  // make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  genfit::AbsTrackRep* motherRepresentation = recoTrackMother->getCardinalRepresentation();
  if ((motherRepresentation == nullptr) || !(recoTrackMother->wasFitSuccessful(motherRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  double motherMass = TDatabasePDG::Instance()->GetParticle(motherRepresentation->getPDG())->Mass();
  double motherCharge = recoTrackMother->getTrackFitStatus()->getCharge();

  // make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  genfit::AbsTrackRep* daughterRepresentation = recoTrackDaughter->getCardinalRepresentation();
  if ((daughterRepresentation == nullptr) || !(recoTrackDaughter->wasFitSuccessful(daughterRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  double daughterMass = TDatabasePDG::Instance()->GetParticle(daughterRepresentation->getPDG())->Mass();
  double daughterCharge = recoTrackDaughter->getTrackFitStatus()->getCharge();

  // make a clone, not use the reference so that the genfit::MeasuredStateOnPlane and its TrackReps will not be altered.
  genfit::MeasuredStateOnPlane stMother = recoTrackMother->getMeasuredStateOnPlaneFromLastHit(motherRepresentation);
  genfit::MeasuredStateOnPlane stDaughter = recoTrackDaughter->getMeasuredStateOnPlaneClosestTo(vertexPosSeed,
                                            daughterRepresentation);
  m_stMotherBuffer = stMother;
  m_stDaughterBuffer = stDaughter;

  // initialize KVertexFitter
  kinkVertexFitterKFit kvf;

  // set magnetic field at the seed position
  const double Bz = BFieldManager::getFieldInTesla({vertexPosSeed.X(), vertexPosSeed.Y(), vertexPosSeed.Z()}).Z();
  kvf.setMagneticField(Bz);

  // set seed position
  HepPoint3D vertexPosSeedHepPoint(vertexPosSeed.X(), vertexPosSeed.Y(), vertexPosSeed.Z());
  kvf.setInitialVertex(vertexPosSeedHepPoint);

  // add mother and daughter states for the fit
  kvf.addState(stMother, motherMass, motherCharge);
  kvf.addState(stDaughter, daughterMass, daughterCharge);

  // do the fit
  int err = kvf.doFit();
  if (err != 0) {
    B2DEBUG(29, "Fit finished with error");
    return false;
  }

  // test the chi2 cut
  if (kvf.getCHIsq() > m_vertexChi2Cut) {
    B2DEBUG(29, "chi^2 too large " << kvf.getCHIsq());
    return false;
  }

  // get the fitted vertex
  HepPoint3D vertexPosHepPoint = kvf.getVertex();
  vertexPos = ROOT::Math::XYZVector(vertexPosHepPoint.x(), vertexPosHepPoint.y(), vertexPosHepPoint.z());

  // extrapolate the mother and the daughter states to the fitted vertex and get the status of the hit reassignment
  if (!extrapolateToVertex(stMother, stDaughter, vertexPos, reassignHitStatus)) {
    B2DEBUG(29, "Failed to extrapolate");
    return false;
  }

  // prepare mother Helix at the fitted vertex to calculate the distance between tracks
  TVector3 motherPos = stMother.getPos();
  TVector3 motherMom = stMother.getMom();
  Helix motherHelix(ROOT::Math::XYZVector(motherPos),
                    ROOT::Math::XYZVector(motherMom),
                    static_cast<short>(recoTrackMother->getTrackFitStatus()->getCharge()),
                    Bz);
  motherHelix.passiveMoveBy(vertexPos);

  // prepare daughter Helix at the fitted vertex to calculate the distance between tracks
  TVector3 daughterPos = stDaughter.getPos();
  TVector3 daughterMom = stDaughter.getMom();
  Helix daughterHelix(ROOT::Math::XYZVector(daughterPos),
                      ROOT::Math::XYZVector(daughterMom),
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



