#include <tracking/v0Finding/fitter/V0Fitter.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/v0Finding/dataobjects/VertexVector.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/trackFitting/trackBuilder/factories/TrackBuilder.h>

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
#include <genfit/GFRaveVertexFactory.h>
#include <genfit/GFRaveVertex.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>
#include <genfit/FitStatus.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>

#include <framework/utilities/IOIntercept.h>

using namespace Belle2;

V0Fitter::V0Fitter(const std::string& trackFitResultsName, const std::string& v0sName,
                   const std::string& v0ValidationVerticesName, const std::string& recoTracksName,
                   const std::string& copiedRecoTracksName, bool enableValidation)
  : m_validation(enableValidation), m_recoTracksName(recoTracksName), m_v0FitterMode(1), m_forcestore(false),
    m_useOnlyOneSVDHitPair(true)
{
  m_trackFitResults.isRequired(trackFitResultsName);
  m_v0s.registerInDataStore(v0sName, DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  //Relation to RecoTracks from Tracks is already tested at the module level.

  if (m_validation) {
    B2DEBUG(24, "Register DataStore for validation.");
    m_validationV0s.registerInDataStore(v0ValidationVerticesName, DataStore::c_ErrorIfAlreadyRegistered);
    m_v0s.registerRelationTo(m_validationV0s);
  }

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

void V0Fitter::setFitterMode(int fitterMode)
{
  if (not(0 <= fitterMode && fitterMode <= 2)) {
    B2FATAL("Invalid fitter mode!");
  } else {
    m_v0FitterMode = fitterMode;
    if (fitterMode == 0)
      m_forcestore = true;
    else
      m_forcestore = false;
    if (fitterMode == 2)
      m_useOnlyOneSVDHitPair = false;
    else
      m_useOnlyOneSVDHitPair = true;
  }
}

void V0Fitter::initializeCuts(double beamPipeRadius,
                              double vertexChi2CutOutside,
                              std::tuple<double, double> invMassRangeKshort,
                              std::tuple<double, double> invMassRangeLambda,
                              std::tuple<double, double> invMassRangePhoton)
{
  m_beamPipeRadius = beamPipeRadius;
  m_vertexChi2CutOutside = vertexChi2CutOutside;
  m_invMassRangeKshort = invMassRangeKshort;
  m_invMassRangeLambda = invMassRangeLambda;
  m_invMassRangePhoton = invMassRangePhoton;
}


bool V0Fitter::fitGFRaveVertex(genfit::Track& trackPlus, genfit::Track& trackMinus, genfit::GFRaveVertex& vertex)
{
  VertexVector vertexVector;
  std::vector<genfit::Track*> trackPair {&trackPlus, &trackMinus};

  try {
    IOIntercept::OutputToLogMessages
    logCapture("V0Fitter GFRaveVertexFactory", LogConfig::c_Debug, LogConfig::c_Debug);
    logCapture.start();

    genfit::GFRaveVertexFactory vertexFactory;
    vertexFactory.findVertices(&vertexVector.v, trackPair);
  } catch (...) {
    B2ERROR("Exception during vertex fit.");
    return false;
  }

  if (vertexVector.size() != 1) {
    B2DEBUG(21, "Vertex fit failed. Size of vertexVector not 1, but: " << vertexVector.size());
    return false;
  }

  if ((*vertexVector[0]).getNTracks() != 2) {
    B2DEBUG(20, "Wrong number of tracks in vertex.");
    return false;
  }

  vertex = *vertexVector[0];
  return true;
}

/// used in the fitAndStore function.
/// if tracks have hits inside the V0 vertex position, bits in hasInnerHitStatus are set.
bool V0Fitter::extrapolateToVertex(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus,
                                   const TVector3& vertexPosition, unsigned int& hasInnerHitStatus)
{
  /// initialize
  hasInnerHitStatus = 0;

  try {
    /// extrapolate the first (innermost) hit to the V0 vertex position
    /// the value will be positive (negative) if the direction of the extrapolation is (counter)momentum-wise
    double extralengthPlus  =  stPlus.extrapolateToPoint(vertexPosition);
    double extralengthMinus = stMinus.extrapolateToPoint(vertexPosition);
    if (extralengthPlus  > 0) hasInnerHitStatus |= 0x1; ///  plus track has hits inside the V0 vertex.
    if (extralengthMinus > 0) hasInnerHitStatus |= 0x2; /// minus track has hits inside the V0 vertex.
  } catch (...) {
    /// This shouldn't ever happen, but I can see the extrapolation
    /// code trying several windings before giving up, so this happens occasionally.
    /// Something more stable would perhaps be desirable.
    B2DEBUG(22, "Could not extrapolate track to vertex.");
    return false;
  }
  return true;
}

double V0Fitter::getBzAtVertex(const TVector3& vertexPosition)
{
  double Bx, By, Bz;
  genfit::FieldManager::getInstance()->getFieldVal(vertexPosition.X(), vertexPosition.Y(), vertexPosition.Z(),
                                                   Bx, By, Bz);
  Bz = Bz / 10;
  return Bz;
}


TrackFitResult* V0Fitter::buildTrackFitResult(const genfit::Track& track, const RecoTrack* recoTrack,
                                              const genfit::MeasuredStateOnPlane& msop, const double Bz,
                                              const Const::ParticleType& trackHypothesis)
{
  const uint64_t hitPatternCDCInitializer = TrackBuilder::getHitPatternCDCInitializer(*recoTrack);
  const uint32_t hitPatternVXDInitializer = TrackBuilder::getHitPatternVXDInitializer(*recoTrack);

  TrackFitResult* v0TrackFitResult
    = m_trackFitResults.appendNew(msop.getPos(), msop.getMom(),
                                  msop.get6DCov(), msop.getCharge(),
                                  trackHypothesis,
                                  track.getFitStatus()->getPVal(),
                                  Bz, hitPatternCDCInitializer, hitPatternVXDInitializer, track.getFitStatus()->getNdf());
  return v0TrackFitResult;
}

std::pair<Const::ParticleType, Const::ParticleType> V0Fitter::getTrackHypotheses(const Const::ParticleType& v0Hypothesis) const
{
  if (v0Hypothesis == Const::Kshort) {
    return std::make_pair(Const::pion, Const::pion);
  } else if (v0Hypothesis == Const::photon) {
    return std::make_pair(Const::electron, Const::electron);
  } else if (v0Hypothesis == Const::Lambda) {
    return std::make_pair(Const::proton, Const::pion);
  } else if (v0Hypothesis == Const::antiLambda) {
    return std::make_pair(Const::pion, Const::proton);
  }
  B2FATAL("Given V0Hypothesis not available.");
  return std::make_pair(Const::invalidParticle, Const::invalidParticle); // return something to avoid triggering cppcheck
}

/// remove hits inside the V0 vertex position
bool V0Fitter::fitAndStore(const Track* trackPlus, const Track* trackMinus,
                           const Const::ParticleType& v0Hypothesis)
{
  /// Existence of corresponding RecoTrack already checked at the module level;
  RecoTrack* recoTrackPlus  =  trackPlus->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackMinus = trackMinus->getRelated<RecoTrack>(m_recoTracksName);

  /// hasInnerHitStatus:
  /// 0x1:  plus track has hits inside the V0 vertex.
  /// 0x2: minus track has hits inside the V0 vertex.
  unsigned int hasInnerHitStatus = 0;

  /// fitted vertex position
  TVector3 vertexPos(0, 0, 0);

  /// Try V0 vertex fit. If the fit fails, return false.
  /// If hasInnerHitStatus evaluated in the vertexFitWithRecoTracks function results to be 0
  /// (no hits inside the V0 vertex position in both tracks),
  /// TrackFitResult and V0 objects are build and stored, and then terminate this function with returning true.
  /// If hasInnerHitStatus!=0, objects are not stored and the tracks are refitted with removing inner hit later.
  if (not vertexFitWithRecoTracks(trackPlus, trackMinus, recoTrackPlus, recoTrackMinus, v0Hypothesis,
                                  hasInnerHitStatus, vertexPos, m_forcestore))
    return false;

  if ((hasInnerHitStatus == 0) or m_forcestore)
    return true;

  /// If one or two tracks have hits inside the V0 vertex.
  /// (0x1(0x2) bit in hasInnerHitStatus represents plus(minus)-track),
  /// redo vertex fit after refitting the track(s) with removing innermost hit in the track(s).
  /// Repeat until all the inner hits are removed or the vertex fit fails.
  bool failflag = false;
  unsigned int nRemoveHitsPlus  = 0;
  unsigned int nRemoveHitsMinus = 0;

  /// PDG code (positive number) used as the track hypothesis in the track fitting.
  const auto trackHypotheses = getTrackHypotheses(v0Hypothesis);
  const int pdg_trackPlus   = trackPlus->getTrackFitResultWithClosestMass(
                                trackHypotheses.first)->getParticleType().getPDGCode();/// positive number
  const int pdg_trackMinus  = trackMinus->getTrackFitResultWithClosestMass(
                                trackHypotheses.second)->getParticleType().getPDGCode();/// positive number

  RecoTrack* recoTrackPlus_forRefit  = copyRecoTrack(recoTrackPlus, pdg_trackPlus);
  RecoTrack* recoTrackMinus_forRefit = copyRecoTrack(recoTrackMinus, pdg_trackMinus);

  if ((recoTrackPlus_forRefit == nullptr) or (recoTrackMinus_forRefit == nullptr))
    return false;

  while (hasInnerHitStatus != 0) {
    /// If the track has a hit inside the V0 vertex position, use refitted RecoTrack with removing inner hits
    /// in the next V0 vertex fit. Else, use the original RecoTrack.

    /// for plus-charged track
    if (hasInnerHitStatus & 0x1) {
      ++nRemoveHitsPlus;
      /// if the track refit fails, break out of this loop and
      /// revert back to the original vertex fit with the original tracks.
      if (not removeInnerHits(recoTrackPlus, recoTrackPlus_forRefit, pdg_trackPlus, nRemoveHitsPlus)) {
        failflag = true;
        break;
      }
    }

    /// for minus-charged track
    if (hasInnerHitStatus & 0x2) {
      ++nRemoveHitsMinus;
      /// if the track refit fails, break out of this loop and
      /// revert back to the original vertex fit with the original tracks.
      if (not removeInnerHits(recoTrackMinus, recoTrackMinus_forRefit, -1 * pdg_trackMinus, nRemoveHitsMinus)) {
        failflag = true;
        break;
      }
    }

    /// V0 vertex fit
    hasInnerHitStatus = 0;
    /// if vertex fit fails, break out of this loop and
    /// revert back to the original vertex fit with the original tracks.
    if (not vertexFitWithRecoTracks(trackPlus, trackMinus, recoTrackPlus_forRefit, recoTrackMinus_forRefit,
                                    v0Hypothesis, hasInnerHitStatus, vertexPos, false)) {
      //B2WARNING("vertex refit failed.");
      failflag = true;
      break;
    }
  }/// end of the while loop

  /// if failflag==true, revert back to the original vertex fit with the original tracks.
  if (failflag) {
    bool forcestore = true;
    if (not vertexFitWithRecoTracks(trackPlus, trackMinus, recoTrackPlus, recoTrackMinus, v0Hypothesis,
                                    hasInnerHitStatus, vertexPos, forcestore)) {
      B2WARNING("original vertex fit fails. this should not happen.");
      return false;
    }
  }

  return true;
}

/// Fit V0 vertex using RecoTracks as inputs.
/// Store V0 and TrackFitResults if hasInnerHitStatus==0 or forceStore==true.
bool V0Fitter::vertexFitWithRecoTracks(const Track* trackPlus, const Track* trackMinus,
                                       RecoTrack* recoTrackPlus, RecoTrack* recoTrackMinus,
                                       const Const::ParticleType& v0Hypothesis,
                                       unsigned int& hasInnerHitStatus, TVector3& vertexPos,
                                       const bool forceStore)
{
  const auto trackHypotheses = getTrackHypotheses(v0Hypothesis);

  genfit::Track gfTrackPlus = RecoTrackGenfitAccess::getGenfitTrack(
                                *recoTrackPlus);/// make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  const int pdgTrackPlus = trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getParticleType().getPDGCode();
  genfit::AbsTrackRep* plusRepresentation = recoTrackPlus->getTrackRepresentationForPDG(pdgTrackPlus);
  if ((plusRepresentation == nullptr) or (not recoTrackPlus->wasFitSuccessful(plusRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  genfit::Track gfTrackMinus = RecoTrackGenfitAccess::getGenfitTrack(
                                 *recoTrackMinus);/// make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  const int pdgTrackMinus = trackMinus->getTrackFitResultWithClosestMass(trackHypotheses.second)->getParticleType().getPDGCode();
  genfit::AbsTrackRep* minusRepresentation = recoTrackMinus->getTrackRepresentationForPDG(pdgTrackMinus);
  if ((minusRepresentation == nullptr) or (not recoTrackMinus->wasFitSuccessful(minusRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  /// If existing, pass to the genfit::Track the correct cardinal representation
  std::vector<genfit::AbsTrackRep*> repsPlus = gfTrackPlus.getTrackReps();
  std::vector<genfit::AbsTrackRep*> repsMinus = gfTrackMinus.getTrackReps();
  if (repsPlus.size() == repsMinus.size()) {
    for (unsigned int id = 0; id < repsPlus.size(); id++) {
      if (abs(repsPlus[id]->getPDG()) == pdgTrackPlus)
        gfTrackPlus.setCardinalRep(id);
      if (abs(repsMinus[id]->getPDG()) == pdgTrackMinus)
        gfTrackMinus.setCardinalRep(id);
    }
  }
  /// The two vectors should always have the same size, and this never happen
  else {
    for (unsigned int id = 0; id < repsPlus.size(); id++) {
      if (abs(repsPlus[id]->getPDG()) == pdgTrackPlus)
        gfTrackPlus.setCardinalRep(id);
    }
    for (unsigned int id = 0; id < repsMinus.size(); id++) {
      if (abs(repsMinus[id]->getPDG()) == pdgTrackMinus)
        gfTrackMinus.setCardinalRep(id);
    }
  }

  /// make a clone, not use the reference so that the genfit::MeasuredStateOnPlane and its TrackReps will not be altered.
  genfit::MeasuredStateOnPlane stPlus  = recoTrackPlus->getMeasuredStateOnPlaneFromFirstHit(plusRepresentation);
  genfit::MeasuredStateOnPlane stMinus = recoTrackMinus->getMeasuredStateOnPlaneFromFirstHit(minusRepresentation);

  genfit::GFRaveVertex vert;
  if (not fitGFRaveVertex(gfTrackPlus, gfTrackMinus, vert)) {
    return false;
  }

  /// Update the tracks (genfit::MeasuredStateOnPlane) after vertex
  ///    fitGFRaveVertex() looks not to give any changes to the input genfit::Track's (I'm not sure..)
  /// make a clone, not use the reference so that the genfit::MeasuredStateOnPlane and its TrackReps will not be altered.
  stPlus  = gfTrackPlus.getFittedState();
  stMinus = gfTrackMinus.getFittedState();

  const TVector3& posVert(vert.getPos());
  vertexPos = posVert;

  /// Apply cuts. We have one set of cuts inside the beam pipe,
  /// the other outside.
  if (posVert.Perp() < m_beamPipeRadius) {
    return false;
  } else {
    if (vert.getChi2() > m_vertexChi2CutOutside) {
      B2DEBUG(22, "Vertex outside beam pipe, chi^2 too large.");
      return false;
    }
  }

  B2DEBUG(22, "Vertex accepted.");

  if (not extrapolateToVertex(stPlus, stMinus, posVert, hasInnerHitStatus)) {
    return false;
  }

  /// store V0
  if (forceStore || hasInnerHitStatus == 0) {
    /// Before storing, apply invariant mass cut.
    const genfit::GFRaveTrackParameters* tr0 = vert.getParameters(0);
    const genfit::GFRaveTrackParameters* tr1 = vert.getParameters(1);
    TLorentzVector lv0, lv1;
    /// Reconstruct invariant mass.
    lv0.SetVectM(tr0->getMom(), trackHypotheses.first.getMass());
    lv1.SetVectM(tr1->getMom(), trackHypotheses.second.getMass());
    double v0InvMass = (lv0 + lv1).M();
    if (v0Hypothesis == Const::Kshort) {
      if (v0InvMass < std::get<0>(m_invMassRangeKshort) || v0InvMass > std::get<1>(m_invMassRangeKshort)) {
        B2DEBUG(22, "Kshort vertex rejected, invariant mass out of range.");
        return false;
      }
    } else if (v0Hypothesis == Const::Lambda || v0Hypothesis == Const::antiLambda) {
      if (v0InvMass < std::get<0>(m_invMassRangeLambda) || v0InvMass > std::get<1>(m_invMassRangeLambda)) {
        B2DEBUG(22, "Lambda vertex rejected, invariant mass out of range.");
        return false;
      }
    } else if (v0Hypothesis == Const::photon) {
      if (v0InvMass < std::get<0>(m_invMassRangePhoton) || v0InvMass > std::get<1>(m_invMassRangePhoton)) {
        B2DEBUG(22, "Photon vertex rejected, invariant mass out of range.");
        return false;
      }
    }


    /// To build the trackFitResult, use the magnetic field at the origin;
    /// the helix is extrapolated to the IP in a constant magnetic field and material effects are neglected
    /// so that the vertexing tool executed on the MDST object will find again this vertex position
    const TVector3 origin(0, 0, 0);
    const double Bz = getBzAtVertex(origin);

    TrackFitResult* tfrPlusVtx =  buildTrackFitResult(gfTrackPlus, recoTrackPlus, stPlus,  Bz, trackHypotheses.first);
    TrackFitResult* tfrMinusVtx = buildTrackFitResult(gfTrackMinus, recoTrackMinus, stMinus, Bz, trackHypotheses.second);

    B2DEBUG(20, "Creating new V0.");
    auto v0 = m_v0s.appendNew(std::make_pair(trackPlus, tfrPlusVtx),
                              std::make_pair(trackMinus, tfrMinusVtx));

    if (m_validation) {
      B2DEBUG(24, "Create StoreArray and Output for validation.");
      auto validationV0 = m_validationV0s.appendNew(
                            std::make_pair(trackPlus, tfrPlusVtx),
                            std::make_pair(trackMinus, tfrMinusVtx),
                            vert.getPos(),
                            vert.getCov(),
                            (lv0 + lv1).P(),
                            (lv0 + lv1).M(),
                            vert.getChi2()
                          );
      v0->addRelationTo(validationV0);
    }
  }
  return true;
}

RecoTrack* V0Fitter::copyRecoTrack(RecoTrack* origRecoTrack, const int trackPDG)
{
  /// original track information
  Const::ChargedStable particleUsedForFitting(std::abs(trackPDG));
  const genfit::AbsTrackRep* origTrackRep = origRecoTrack->getTrackRepresentationForPDG(std::abs(
                                              trackPDG));/// only a positive PDG number is allowed for the input

  RecoTrack* newRecoTrack = origRecoTrack->copyToStoreArray(m_copiedRecoTracks);
  newRecoTrack->addHitsFromRecoTrack(origRecoTrack);
  newRecoTrack->addRelationTo(origRecoTrack);

  /// fit newRecoTrack
  TrackFitter fitter;
  if (not fitter.fit(*newRecoTrack, particleUsedForFitting)) {
    B2WARNING("track fit failed for copied RecoTrack.");
    /// check fit status of original track
    if (not origRecoTrack->wasFitSuccessful(origTrackRep))
      B2WARNING("\t original track fit was also failed.");
    return nullptr;
  }

  return newRecoTrack;
}

bool V0Fitter::removeInnerHits(RecoTrack* origRecoTrack, RecoTrack* recoTrack,
                               const int trackPDG, unsigned int& nRemoveHits)
{
  /// original track information
  Const::ChargedStable particleUsedForFitting(std::abs(trackPDG));
  const genfit::AbsTrackRep* origTrackRep = origRecoTrack->getTrackRepresentationForPDG(std::abs(
                                              trackPDG));/// only a positive PDG number is allowed for the input

  /// disable inner hits
  const std::vector<RecoHitInformation*>& recoHitInformations = recoTrack->getRecoHitInformations(true);/// true for sorted info.
  if (recoHitInformations.size() < nRemoveHits) {/// N removed hits should not exceed N hits in the track
    B2WARNING("N removed hits exceeds N hits in the track.");
    return false;
  }
  for (unsigned int i = 0 ; i < nRemoveHits ; ++i) {
    recoHitInformations[i]->setUseInFit(false);
  }
  /// if the last removed hit is a SVD U-hit, remove the next hit (SVD V-hit as the pair) in addition
  ///    note: for the SVD pair hits, U-hit should be first and the V-hit the next in the sorted RecoHitInformation array.
  if (recoHitInformations[nRemoveHits - 1]->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_SVD) {
    if (recoHitInformations.size() < nRemoveHits + 1) { /// N removed hits should not exceed N hits in the track
      B2WARNING("N removed hits exceeds N hits in the track.");
      return false;
    }
    if (recoHitInformations[nRemoveHits]->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_SVD) {
      const SVDCluster* lastRemovedSVDHit = recoHitInformations[nRemoveHits - 1]->getRelatedTo<SVDCluster>();
      const SVDCluster* nextSVDHit        = recoHitInformations[nRemoveHits]  ->getRelatedTo<SVDCluster>();
      if (lastRemovedSVDHit->getSensorID() == nextSVDHit->getSensorID() &&
          lastRemovedSVDHit->isUCluster() && !(nextSVDHit->isUCluster())) {
        recoHitInformations[nRemoveHits]->setUseInFit(false);
        ++nRemoveHits;
      }
    }
  }

  /// if N of remaining SVD hit-pair is only one, don't use the SVD hits
  if (!m_useOnlyOneSVDHitPair &&
      recoHitInformations[nRemoveHits - 1]->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_SVD &&
      recoHitInformations.size() > nRemoveHits + 2) {
    if (recoHitInformations[nRemoveHits  ]->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_SVD && /// SVD U-hit
        recoHitInformations[nRemoveHits + 1]->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_SVD && /// SVD V-hit
        recoHitInformations[nRemoveHits + 2]->getTrackingDetector() != RecoHitInformation::RecoHitDetector::c_SVD) { /// not SVD hit (CDC)
      recoHitInformations[nRemoveHits  ]->setUseInFit(false);
      recoHitInformations[nRemoveHits + 1]->setUseInFit(false);
      nRemoveHits += 2;
    }
  }

  /// fit recoTrack
  TrackFitter fitter;
  if (not fitter.fit(*recoTrack, particleUsedForFitting)) {
    B2WARNING("track fit failed.");
    /// check fit status of original track
    if (not origRecoTrack->wasFitSuccessful(origTrackRep))
      B2WARNING("\t original track fit was also failed.");
    return false;
  }

  return true;
}

int V0Fitter::checkSharedInnermostCluster(const RecoTrack* recoTrackPlus, const RecoTrack* recoTrackMinus)
{
  int flag = 0; // 1 for sharing 1D-hit, 2 for sharing 2D-hit

  for (auto& recoHitInfoPlus : recoTrackPlus->getRecoHitInformations(true)) {
    if (not recoHitInfoPlus->useInFit()) continue; // get the innermost hit for plus-daughter
    for (auto& recoHitInfoMinus : recoTrackMinus->getRecoHitInformations(true)) {
      if (not recoHitInfoMinus->useInFit()) continue; // get the innermost hit for minus-daughter
      if (recoHitInfoPlus->getTrackingDetector() == recoHitInfoMinus->getTrackingDetector()) {
        if (recoHitInfoPlus->getTrackingDetector() == RecoHitInformation::c_PXD) {
          const PXDCluster* clusterPlus = recoHitInfoPlus->getRelatedTo<PXDCluster>();
          const PXDCluster* clusterMinus = recoHitInfoMinus->getRelatedTo<PXDCluster>();
          if (clusterPlus == clusterMinus) { // if they share a same PXDCluster, set the flag
            flag = 2; // PXD cluster is a 2D-hit
            break;
          }
        } else if (recoHitInfoPlus->getTrackingDetector() == RecoHitInformation::c_SVD) {
          const SVDCluster* clusterPlus = recoHitInfoPlus->getRelatedTo<SVDCluster>();
          const SVDCluster* clusterMinus = recoHitInfoMinus->getRelatedTo<SVDCluster>();
          if (clusterPlus == clusterMinus) {
            /// if the innermost hit is a SVD U-hit and shared, check the next hit (SVD V-hit as the pair) in addition
            ///    note: for the SVD pair hits, U-hit should be first and the V-hit the next in the sorted RecoHitInformation array.
            if (clusterPlus->isUCluster()) {
              const auto& recoHitInfoNextPlus  = ++recoHitInfoPlus;
              const auto& recoHitInfoNextMinus = ++recoHitInfoMinus;
              if (recoHitInfoNextPlus->useInFit() && recoHitInfoNextMinus->useInFit()  // this should be satisfied by default
                  && recoHitInfoNextPlus->getTrackingDetector() == recoHitInfoNextMinus->getTrackingDetector()
                  && recoHitInfoNextPlus->getTrackingDetector() == RecoHitInformation::c_SVD) {
                const SVDCluster* clusterPlusV = recoHitInfoNextPlus->getRelatedTo<SVDCluster>();
                const SVDCluster* clusterMinusV = recoHitInfoNextMinus->getRelatedTo<SVDCluster>();
                if (clusterPlus->getSensorID() == clusterPlusV->getSensorID()
                    && clusterPlusV == clusterMinusV && !(clusterPlusV->isUCluster())) {
                  flag = 2; // SVD U- and V-cluster gives 2D-hit information
                  break;
                }
              }
            }
            flag = 1; // share SVD U/V-cluster only, 1D-hit
            break;
          }
        } else break;
      }
    }
    break;
  }
  return flag;

}
