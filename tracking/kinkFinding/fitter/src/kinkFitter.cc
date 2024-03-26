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



  B2ASSERT("Material effects not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::MaterialEffects::getInstance()->isInitialized());
  B2ASSERT("Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::FieldManager::getInstance()->isInitialized());
}

void kinkFitter::setFitterMode(int fitterMode)
{
  if (not(0 <= fitterMode && fitterMode <= 5)) {
    B2FATAL("Invalid fitter mode!");
  } else {
    m_kinkFitterMode = fitterMode;
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
      try {
        const genfit::MeasuredStateOnPlane& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromRecoHit(recoHitInfo);
        const double currentDistance2 = (ROOT::Math::XYZVector(measuredStateOnPlane.getPos()) - vertexPos).Mag2();

        if (not minimalDistance2 or currentDistance2 < minimalDistance2) {
          minimalDistance2 = currentDistance2;
          minimalIndex = ri;
        }
        // if cannot find minimum in 3 iterations, stop searching
        if (ri - minimalIndex > 3) break;
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
      try {
        const genfit::MeasuredStateOnPlane& measuredStateOnPlane = recoTrack->getMeasuredStateOnPlaneFromRecoHit(recoHitInfo);
        const double currentDistance2 = (ROOT::Math::XYZVector(measuredStateOnPlane.getPos()) - vertexPos).Mag2();

        if (not minimalDistance2 or currentDistance2 < minimalDistance2) {
          minimalDistance2 = currentDistance2;
          minimalIndex = ri;
        }
        // if cannot find minimum in 3 iterations, stop searching
        if (minimalIndex - ri > 3) break;
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
    B2INFO("Fit failed ");
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

  // DAF fitter usually bad with flipped tracks, so kalmanFitter is used instead
  std::shared_ptr<genfit::KalmanFitterRefTrack> kalmanFitter = std::make_shared<genfit::KalmanFitterRefTrack>();
  kalmanFitter->setMinIterations(static_cast<unsigned int>(3));
  kalmanFitter->setMaxIterations(static_cast<unsigned int>(10));
  kalmanFitter->setMaxFailedHits(static_cast<unsigned int>(5));

  trackFitter.resetFitter(kalmanFitter);

  RecoTrack* newRecoTrack = m_copiedRecoTracks.appendNew(positionSeed,
                                                         -momentumSeed,
                                                         static_cast<short>(-recoTrack->getChargeSeed()));
  newRecoTrack->setTimeSeed(timeSeed);
  newRecoTrack->setSeedCovariance(recoTrack->getSeedCovariance());
  newRecoTrack->addHitsFromRecoTrack(recoTrack, newRecoTrack->getNumberOfTotalHits(), true);

  trackFitter.fit(*newRecoTrack);

  return newRecoTrack;
}

/// Fit and store kink.
/// If the fitterMode requires, tries to reassign hits between daughter and mother tracks.
/// If the fitterMode requires, tries to flip and refit the tracks.
bool kinkFitter::fitAndStore(const Track* trackMother, const Track* trackDaughter, short filterFlag)
{

  //if (filterFlag != 2) return false;

  // Existence of corresponding RecoTrack already checked at the module level;
  RecoTrack* recoTrackMother = trackMother->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackDaughter = trackDaughter->getRelated<RecoTrack>(m_recoTracksName);

  // fitted vertex position
  ROOT::Math::XYZVector vertexPos(0, 0, 0);

  // flag to reassign hits, final hit to reassign, and distance at the fitted vertex
  unsigned int reassignHitStatus = 0;
  int finalHitPositionForReassignment = 0;
  double distanceAtVertex = 0;

  // Try kink vertex fit. If the fit fails, return false.
  if (!vertexFitWithRecoTracks(recoTrackMother, recoTrackDaughter, reassignHitStatus, vertexPos, distanceAtVertex,
                               ROOT::Math::XYZVector(recoTrackMother->getMeasuredStateOnPlaneFromLastHit().getPos())))
    return false;

  // Mother and daughter States extrapolated to the fitted vertex
  genfit::MeasuredStateOnPlane stMother = m_stMotherBuffer;
  genfit::MeasuredStateOnPlane stDaughter = m_stDaughterBuffer;

  // recoTracks used to fit the vertex
  m_motherKinkRecoTrackCache = recoTrackMother;
  m_daughterKinkRecoTrackCache = recoTrackDaughter;

  // if the corresponding fitterMode is used, try to reassign hits between mother and daughter tracks
  if ((m_kinkFitterMode % 2 == 1) && (filterFlag == 1 || filterFlag == 4) && (reassignHitStatus != 0)) {

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

      // find threshold hit position in the track
      // hit bellow the threshold are to be reassigned
      // positive for daughter hits reassignment to mother, negative vice-versa
      int hitPositionForReassignment = 0;
      if (reassignHitStatus & 0x1) {

        // daughter hits to be reassigned
        /*hitPositionForReassignment = findHitPositionForReassignment(recoTrackDaughter, vertexPosTmp, 1);*/
        hitPositionForReassignment = findHitPositionForReassignment(recoTrackDaughterBuffer, vertexPosTmp, 1);

        // test if the number of hits to be reassigned larger than the number of CDC hits in daughter tracks
        // minus number degree of freedom required for the fit
        /*if (static_cast<unsigned int>(-hitPositionForReassignment + 5) >
            recoTrackDaughter->getNumberOfCDCHits())
            break;*/
        if (static_cast<unsigned int>(-hitPositionForReassignment + 5) >
            recoTrackDaughterBuffer->getNumberOfCDCHits())
          break;

      } else if (reassignHitStatus & 0x2) {

        // mother hits to be reassigned
        /*hitPositionForReassignment = findHitPositionForReassignment(recoTrackMother, vertexPosTmp, -1);*/
        hitPositionForReassignment = findHitPositionForReassignment(recoTrackMotherBuffer, vertexPosTmp, -1);

        // test if the number of hits to be reassigned larger than the number of CDC hits in mother tracks
        // minus number degree of freedom required for the fit
        /*if (static_cast<unsigned int>(hitPositionForReassignment + 5) > recoTrackMother->getNumberOfCDCHits()) {

            // if number of SVD hits is enough for the fit, reassign all the CDC hits
            if (recoTrackMother->getNumberOfSVDHits() > 5)
                hitPositionForReassignment = recoTrackMother->getNumberOfCDCHits();
            else
                break;

        }*/
        if (static_cast<unsigned int>(hitPositionForReassignment + 5) > recoTrackMotherBuffer->getNumberOfCDCHits()) {

          // if number of SVD hits is enough for the fit, reassign all the CDC hits
          if (recoTrackMotherBuffer->getNumberOfSVDHits() > 5)
            hitPositionForReassignment = recoTrackMotherBuffer->getNumberOfCDCHits();
          else
            break;

        }
      }
      B2DEBUG(29, "Found hit index to reassign " << hitPositionForReassignment);

      // if the found hit position is the same as at previous iteration, give up
      /*if (hitPositionForReassignment == previousHitPositionForReassignment)
          break;
      previousHitPositionForReassignment = hitPositionForReassignment;*/

      // refit of the new tracks can fail when the position is too far
      // try positions closer to the end until reach it
      while (abs(hitPositionForReassignment) > 0) {

        // create new RecoTracks with reassigned hits in the separate StoreArray
        /*recoTrackMotherRefit = copyRecoTrackAndReassignCDCHits(recoTrackMother,
                                                               recoTrackDaughter,
                                                               true, hitPositionForReassignment);
        recoTrackDaughterRefit = copyRecoTrackAndReassignCDCHits(recoTrackMother,
                                                                 recoTrackDaughter,
                                                                 false, hitPositionForReassignment);*/
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
        B2DEBUG(29, "Fit of the tracks failed, try with smaller hit index: " << hitPositionForReassignment);
      }

      // if hit position reaches end, the trial failed. Continue with default tracks.
      if (hitPositionForReassignment == 0) {
        B2DEBUG(29, "Reassign hits and refit failed");
        break;
      }

      // Try to fit vertex, using previous result as a seed. If the fit fails, continue with default tracks.
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

  // if the corresponding fitterMode is used, try to flip the daughter track and refit it
  if ((m_kinkFitterMode == 2 || m_kinkFitterMode == 3) && (filterFlag == 2)) {

    B2INFO("Try to flip and refit");

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

        B2INFO("Flip and refit successful");
      }
  }

  B2INFO("Distance between tracks at fitted vertex " << distanceAtVertex);
  B2INFO("Radius of the vertex " << vertexPos.Rho());
  B2INFO("Reassigned hit " << finalHitPositionForReassignment);

  // test the distance cut
  if (distanceAtVertex > m_vertexDistanceCut)
    return false;

  // check if the fitted vertex is inside CDC
  if (vertexPos.Rho() < 17)
    return false;

  // temporary StoreArrays for studies (to be removed)
  RecoTrack* motherKinkRecoTrack = recoTrackMother->copyToStoreArray(m_motherKinkRecoTracks);
  motherKinkRecoTrack->addHitsFromRecoTrack(recoTrackMother, motherKinkRecoTrack->getNumberOfTotalHits());
  motherKinkRecoTrack->addRelationTo(recoTrackMother);

  RecoTrack* daughterKinkRecoTrack = recoTrackDaughter->copyToStoreArray(m_daughterKinkRecoTracks);
  daughterKinkRecoTrack->addHitsFromRecoTrack(recoTrackDaughter, daughterKinkRecoTrack->getNumberOfTotalHits());
  daughterKinkRecoTrack->addRelationTo(recoTrackDaughter);

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
  genfit::Track gfTrackMother = RecoTrackGenfitAccess::getGenfitTrack(*recoTrackMother);
  genfit::AbsTrackRep* motherRepresentation = recoTrackMother->getCardinalRepresentation();
  if ((motherRepresentation == nullptr) || !(recoTrackMother->wasFitSuccessful(motherRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  double motherMass = TDatabasePDG::Instance()->GetParticle(motherRepresentation->getPDG())->Mass();
  double motherCharge = recoTrackMother->getTrackFitStatus()->getCharge();

  // make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  genfit::Track gfTrackDaughter = RecoTrackGenfitAccess::getGenfitTrack(*recoTrackDaughter);
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
    B2INFO("Fit finished with error");
    return false;
  }

  // test the chi2 cut
  if (kvf.getCHIsq() > m_vertexChi2Cut) {
    B2INFO("chi^2 too large " << kvf.getCHIsq());
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



