#include <tracking/v0Finding/fitter/V0Fitter.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/v0Finding/dataobjects/VertexVector.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

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
  : m_validation(enableValidation), m_recoTracksName(recoTracksName), m_v0FitterMode(0)
{
  m_trackFitResults.isRequired(trackFitResultsName);
  m_v0s.registerInDataStore(v0sName, DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  //Relation to RecoTracks from Tracks is already tested at the module level.

  if (m_validation) {
    B2DEBUG(300, "Register DataStore for validation.");
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


  B2ASSERT(genfit::MaterialEffects::getInstance()->isInitialized(),
           "Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  B2ASSERT(genfit::FieldManager::getInstance()->isInitialized(),
           "Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.");
}

void V0Fitter::setFitterMode(int fitterMode)
{
  if (not(0 <= fitterMode && fitterMode <= 2)) {
    B2WARNING("invarid fitter mode! set as the default fitter");
  } else {
    m_v0FitterMode = fitterMode;
  }
}

void V0Fitter::initializeCuts(double beamPipeRadius,
                              double vertexChi2CutOutside)
{
  m_beamPipeRadius = beamPipeRadius;
  m_vertexChi2CutOutside = vertexChi2CutOutside;
}


bool V0Fitter::rejectCandidate(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus)
{
  const TVector3& posPlus = stPlus.getPos();
  const TVector3& posMinus = stMinus.getPos();

  const double Rstart = std::min(posPlus.Perp(), posMinus.Perp());
  try {
    stPlus.extrapolateToCylinder(Rstart);
    stMinus.extrapolateToCylinder(Rstart);
  } catch (...) {
    B2DEBUG(200, "Extrapolation to cylinder failed.");
    return true;
  }

  return false;
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
    B2DEBUG(150, "Vertex fit failed. Size of vertexVector not 1, but: " << vertexVector.size());
    return false;
  }

  if ((*vertexVector[0]).getNTracks() != 2) {
    B2DEBUG(100, "Wrong number of tracks in vertex.");
    return false;
  }

  vertex = *vertexVector[0];
  return true;
}

/// used in the fitAndStore0 function.
bool V0Fitter::extrapolateToVertex(genfit::MeasuredStateOnPlane& stPlus, genfit::MeasuredStateOnPlane& stMinus,
                                   const TVector3& vertexPosition)
{
  try {
    stPlus.extrapolateToPoint(vertexPosition);
    stMinus.extrapolateToPoint(vertexPosition);
  } catch (...) {
    // This shouldn't ever happen, but I can see the extrapolation
    // code trying several windings before giving up, so this
    // happens occasionally.  Something more stable would perhaps
    // be desirable.
    B2DEBUG(200, "Could not extrapolate track to vertex.");
    return false;
  }
  return true;
}

/// used in the fitAndStore2 function.
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
    B2DEBUG(200, "Could not extrapolate track to vertex.");
    //B2WARNING("extrapolateToPoint was failed");
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


TrackFitResult* V0Fitter::buildTrackFitResult(const genfit::Track& track, const genfit::MeasuredStateOnPlane& msop, const double Bz,
                                              const Const::ParticleType& trackHypothesis)
{
  if (m_v0FitterMode == 1 || m_v0FitterMode == 2) {
    const uint64_t hitPatternCDCInitializer = getHitPatternCDCInitializer(track);
    const uint32_t hitPatternVXDInitializer = getHitPatternVXDInitializer(track);

    TrackFitResult* v0TrackFitResult
      = m_trackFitResults.appendNew(msop.getPos(), msop.getMom(),
                                    msop.get6DCov(), msop.getCharge(),
                                    trackHypothesis,
                                    track.getFitStatus()->getPVal(),
                                    Bz, hitPatternCDCInitializer, hitPatternVXDInitializer);
    return v0TrackFitResult;
  } else {
    TrackFitResult* v0TrackFitResult
      = m_trackFitResults.appendNew(msop.getPos(), msop.getMom(),
                                    msop.get6DCov(), msop.getCharge(),
                                    trackHypothesis,
                                    track.getFitStatus()->getPVal(),
                                    Bz, 0, 0);
    return v0TrackFitResult;
  }
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

/// original fitAndStore function
bool V0Fitter::fitAndStore0(const Track* trackPlus, const Track* trackMinus,
                            const Const::ParticleType& v0Hypothesis)
{
  const auto trackHypotheses = getTrackHypotheses(v0Hypothesis);

  //Existence of corresponding RecoTrack already checked at the module level;
  RecoTrack* recoTrackPlus = trackPlus->getRelated<RecoTrack>(m_recoTracksName);
  genfit::Track gfTrackPlus = RecoTrackGenfitAccess::getGenfitTrack(
                                *recoTrackPlus);/// make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  int pdgTrackPlus = trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getParticleType().getPDGCode();
  genfit::AbsTrackRep* plusRepresentation = recoTrackPlus->getTrackRepresentationForPDG(pdgTrackPlus);
  if ((plusRepresentation == nullptr) or (not recoTrackPlus->wasFitSuccessful(plusRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  //Existence of corresponding RecoTrack already checked at the module level;
  RecoTrack* recoTrackMinus = trackMinus->getRelated<RecoTrack>(m_recoTracksName);
  genfit::Track gfTrackMinus = RecoTrackGenfitAccess::getGenfitTrack(
                                 *recoTrackMinus);/// make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  int pdgTrackMinus = trackMinus->getTrackFitResultWithClosestMass(trackHypotheses.second)->getParticleType().getPDGCode();
  genfit::AbsTrackRep* minusRepresentation = recoTrackMinus->getTrackRepresentationForPDG(pdgTrackMinus);
  if ((minusRepresentation == nullptr) or (not recoTrackMinus->wasFitSuccessful(minusRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  // If existing, pass to the genfit::Track the correct cardinal representation
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
  // The two vectors should always have the same size, and this never happen
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
  genfit::MeasuredStateOnPlane stPlus = recoTrackPlus->getMeasuredStateOnPlaneFromFirstHit(plusRepresentation);
  genfit::MeasuredStateOnPlane stMinus = recoTrackMinus->getMeasuredStateOnPlaneFromFirstHit(minusRepresentation);

  if (rejectCandidate(stPlus, stMinus)) {
    return false;
  }

  genfit::GFRaveVertex vert;
  if (not fitGFRaveVertex(gfTrackPlus, gfTrackMinus, vert)) {
    return false;
  }

  // Update the tracks (genfit::MeasuredStateOnPlane) after vertex
  //    fitGFRaveVertex() looks not to give any changes to the input genfit::Track's (not sure..)
  /// make a clone, not use the reference so that the genfit::MeasuredStateOnPlane and its TrackReps will not be altered.
  stPlus  = gfTrackPlus.getFittedState();
  stMinus = gfTrackMinus.getFittedState();

  ///// these two lines can be moved to inside the "if(m_validation){..." parenthesis ?
  const genfit::GFRaveTrackParameters* tr0 = vert.getParameters(0);
  const genfit::GFRaveTrackParameters* tr1 = vert.getParameters(1);

  const TVector3& posVert(vert.getPos());

  // Apply cuts. We have one set of cuts inside the beam pipe,
  // the other outside.
  if (posVert.Perp() < m_beamPipeRadius) {
    return false;

  } else {
    if (vert.getChi2() > m_vertexChi2CutOutside) {
      B2DEBUG(200, "Vertex outside beam pipe, chi^2 too large.");
      return false;
    }
  }

  B2DEBUG(200, "Vertex accepted.");

  // Assemble V0s.
  if (not extrapolateToVertex(stPlus, stMinus, posVert)) {
    return false;
  }

  // To build the trackFitResult use the magnetic field at the origin;
  // the helix is extrapolated to the IP in a constant magnetic field and material effects are neglected
  // so that the vertexing tool executed on the MDST object will find again this vertex position
  const TVector3 origin(0, 0, 0);
  const double Bz = getBzAtVertex(origin);

  TrackFitResult* tfrPlusVtx = buildTrackFitResult(gfTrackPlus, stPlus, Bz, trackHypotheses.first);
  TrackFitResult* tfrMinusVtx = buildTrackFitResult(gfTrackMinus, stMinus, Bz, trackHypotheses.second);

  B2DEBUG(100, "Creating new V0.");
  auto v0 = m_v0s.appendNew(std::make_pair(trackPlus, tfrPlusVtx),
                            std::make_pair(trackMinus, tfrMinusVtx));

  if (m_validation) {
    B2DEBUG(300, "Create StoreArray and Output for validation.");
    TLorentzVector lv0, lv1;
    // Reconstruct invariant mass.
    lv0.SetVectM(tr0->getMom(), trackHypotheses.first.getMass());
    lv1.SetVectM(tr1->getMom(), trackHypotheses.second.getMass());
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
  return true;
}

/// original with vertexFitWithRecoTracks function
bool V0Fitter::fitAndStore1(const Track* trackPlus, const Track* trackMinus,
                            const Const::ParticleType& v0Hypothesis)
{
  RecoTrack* recoTrackPlus  =  trackPlus->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackMinus = trackMinus->getRelated<RecoTrack>(m_recoTracksName);

  unsigned int hasInnerHitStatus = 0;
  const bool forcestore = true;
  TVector3 vertexPos(0, 0, 0);
  return vertexFitWithRecoTracks(trackPlus, trackMinus, recoTrackPlus, recoTrackMinus, v0Hypothesis,
                                 hasInnerHitStatus, vertexPos, forcestore);
}

/// remove hits inside the V0 vertex position
bool V0Fitter::fitAndStore2(const Track* trackPlus, const Track* trackMinus,
                            const Const::ParticleType& v0Hypothesis)
{
  const auto trackHypotheses = getTrackHypotheses(v0Hypothesis);
  RecoTrack* recoTrackPlus  =  trackPlus->getRelated<RecoTrack>(m_recoTracksName);
  RecoTrack* recoTrackMinus = trackMinus->getRelated<RecoTrack>(m_recoTracksName);

  /// PDG code (positive number) used as the track hypothesis in the track fitting.
  const int pdg_trackPlus   = trackPlus->getTrackFitResultWithClosestMass(
                                trackHypotheses.first)->getParticleType().getPDGCode();/// positive number
  const int pdg_trackMinus  = trackMinus->getTrackFitResultWithClosestMass(
                                trackHypotheses.second)->getParticleType().getPDGCode();/// positive number

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
                                  hasInnerHitStatus, vertexPos, false))
    return false;

  if (hasInnerHitStatus == 0)
    return true;

  /// If one or two tracks have hits inside the V0 vertex.
  /// (0x1(0x2) bit in hasInnerHitStatus represents plus(minus)-track),
  /// redo vertex fit after refitting the track(s) with removing innermost hit in the track(s).
  /// Repeat until all the inner hits are removed or the vertex fit fails.
  bool failflag = false;
  unsigned int nRemoveHitsPlus  = 0;
  unsigned int nRemoveHitsMinus = 0;

  RecoTrack* recoTrackPlus_forRefit  = recoTrackPlus;
  RecoTrack* recoTrackMinus_forRefit = recoTrackMinus;

  while (hasInnerHitStatus != 0) {
    /// If the track has a hit inside the V0 vertex position, use refitted RecoTrack with removing inner hits
    /// in the next V0 vertex fit. Else, use the original RecoTrack.

    /// for plus-charged track
    if (hasInnerHitStatus & 0x1) {
      ++nRemoveHitsPlus;
      recoTrackPlus_forRefit = copyRecoTrackRemovingInnerHits(trackPlus, recoTrackPlus,
                                                              pdg_trackPlus, nRemoveHitsPlus);
      /// if the track refit fails, break out of this loop and
      /// revert back to the original vertex fit with the original tracks.
      if (recoTrackPlus_forRefit == nullptr) {
        failflag = true;
        break;
      }
    }

    /// for minus-charged track
    if (hasInnerHitStatus & 0x2) {
      ++nRemoveHitsMinus;
      recoTrackMinus_forRefit = copyRecoTrackRemovingInnerHits(trackMinus, recoTrackMinus,
                                                               -1 * pdg_trackMinus, nRemoveHitsMinus);
      /// if the track refit fails, break out of this loop and
      /// revert back to the original vertex fit with the original tracks.
      if (recoTrackMinus_forRefit == nullptr) {
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
    if (vertexFitWithRecoTracks(trackPlus, trackMinus, recoTrackPlus, recoTrackMinus, v0Hypothesis,
                                hasInnerHitStatus, vertexPos, forcestore)) {
      recoTrackPlus_forRefit  = recoTrackPlus;
      recoTrackMinus_forRefit = recoTrackMinus;
    } else {
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

  /// Existence of corresponding RecoTrack already checked at the module level;
  genfit::Track gfTrackPlus = RecoTrackGenfitAccess::getGenfitTrack(
                                *recoTrackPlus);/// make a clone, not use the reference so that the genfit::Track and its TrackReps will not be altered.
  const int pdgTrackPlus = trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getParticleType().getPDGCode();
  genfit::AbsTrackRep* plusRepresentation = recoTrackPlus->getTrackRepresentationForPDG(pdgTrackPlus);
  if ((plusRepresentation == nullptr) or (not recoTrackPlus->wasFitSuccessful(plusRepresentation))) {
    B2ERROR("Track hypothesis with closest mass not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  /// Existence of corresponding RecoTrack already checked at the module level;
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

  if (rejectCandidate(stPlus, stMinus)) {
    return false;
  }

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
      B2DEBUG(200, "Vertex outside beam pipe, chi^2 too large.");
      return false;
    }
  }

  B2DEBUG(200, "Vertex accepted.");

  if (not extrapolateToVertex(stPlus, stMinus, posVert, hasInnerHitStatus)) {
    return false;
  }

  /// store V0
  if (forceStore || hasInnerHitStatus == 0) {
    /// To build the trackFitResult, use the magnetic field at the origin;
    /// the helix is extrapolated to the IP in a constant magnetic field and material effects are neglected
    /// so that the vertexing tool executed on the MDST object will find again this vertex position
    const TVector3 origin(0, 0, 0);
    const double Bz = getBzAtVertex(origin);

    TrackFitResult* tfrPlusVtx =  buildTrackFitResult(gfTrackPlus, stPlus,  Bz, trackHypotheses.first);
    TrackFitResult* tfrMinusVtx = buildTrackFitResult(gfTrackMinus, stMinus, Bz, trackHypotheses.second);

    B2DEBUG(100, "Creating new V0.");
    auto v0 = m_v0s.appendNew(std::make_pair(trackPlus, tfrPlusVtx),
                              std::make_pair(trackMinus, tfrMinusVtx));

    if (m_validation) {
      B2DEBUG(300, "Create StoreArray and Output for validation.");
      const genfit::GFRaveTrackParameters* tr0 = vert.getParameters(0);
      const genfit::GFRaveTrackParameters* tr1 = vert.getParameters(1);
      TLorentzVector lv0, lv1;
      /// Reconstruct invariant mass.
      lv0.SetVectM(tr0->getMom(), trackHypotheses.first.getMass());
      lv1.SetVectM(tr1->getMom(), trackHypotheses.second.getMass());
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

RecoTrack* V0Fitter::copyRecoTrackRemovingInnerHits(const Track* origTrack, RecoTrack* origRecoTrack,
                                                    const int trackPDG, unsigned int& nRemoveHits)
{
  /// original track information
  Const::ChargedStable particleUsedForFitting(std::abs(trackPDG));
  const genfit::AbsTrackRep* origTrackRep = origRecoTrack->getTrackRepresentationForPDG(std::abs(
                                              trackPDG));/// only a positive PDG number is allowed for the input

  RecoTrack* newRecoTrack = origRecoTrack->copyToStoreArray(m_copiedRecoTracks);
  newRecoTrack->addHitsFromRecoTrack(origRecoTrack);
  newRecoTrack->addRelationTo(origRecoTrack);

  /// disable inner hits
  const std::vector<RecoHitInformation*>& recoHitInformations = newRecoTrack->getRecoHitInformations(true);/// true for sorted info.
  if (recoHitInformations.size() < nRemoveHits) {/// N removed hits should not exceed N hits in the track
    B2WARNING("N removed hits exceeds N hits in the track.");
    return nullptr;
  }
  for (unsigned int i = 0 ; i < nRemoveHits ; ++i) {
    recoHitInformations[i]->setUseInFit(false);
  }
  /// if the last removed hit is a SVD U-hit, remove the next hit (SVU V-hit as the pair) in addition
  ///    note: for the SVD pair hits, U-hit should be first and the V-hit the next in the sorted RecoHitInformation array.
  if (recoHitInformations[nRemoveHits - 1]->getTrackingDetector() == RecoHitInformation::RecoHitDetector::c_SVD) {
    if (recoHitInformations.size() < nRemoveHits + 1) { /// N removed hits should not exceed N hits in the track
      B2WARNING("N removed hits exceeds N hits in the track.");
      return nullptr;
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

  /// fit newRecoTrack
  TrackFitter fitter;
  if (not fitter.fit(*newRecoTrack, particleUsedForFitting)) {
    B2WARNING("track fit failed.");
    /// check fit statust of original track
    if (not origRecoTrack->wasFitSuccessful(origTrackRep))
      B2WARNING("\t original track fit was also failed.");
    return nullptr;
  }

  return newRecoTrack;
}

///// copy from TrackBuilder::getHitPatternVXDInitializer
uint32_t V0Fitter::getHitPatternVXDInitializer(const genfit::Track& track) const
{
  HitPatternVXD hitPatternVXD;

  const auto& hitPointsWithMeasurements = track.getPointsWithMeasurement();
  int nNotFittedVXDhits = 0;

  for (const auto& trackPoint : hitPointsWithMeasurements) {

    for (size_t measurementId = 0; measurementId < trackPoint->getNumRawMeasurements(); measurementId++) {

      genfit::AbsMeasurement* absMeas = trackPoint->getRawMeasurement(measurementId);
      genfit::KalmanFitterInfo* kalmanInfo = trackPoint->getKalmanFitterInfo();

      if (kalmanInfo) {
        const double weight = kalmanInfo->getWeights().at(measurementId);
        if (weight == 0)
          continue;
      } else {
        ++nNotFittedVXDhits;
        continue;
      }

      PXDRecoHit* pxdHit = dynamic_cast<PXDRecoHit*>(absMeas);
      if (pxdHit) {
        const int layerNumber = pxdHit->getSensorID().getLayerNumber();
        const int currentHits = hitPatternVXD.getPXDLayer(layerNumber, HitPatternVXD::PXDMode::normal);
        hitPatternVXD.setPXDLayer(layerNumber, currentHits + 1, HitPatternVXD::PXDMode::normal);
      }

      SVDRecoHit* svdHit = dynamic_cast<SVDRecoHit*>(absMeas);
      SVDRecoHit2D* svdHit2D = dynamic_cast<SVDRecoHit2D*>(absMeas);
      if (svdHit2D) {
        const int layerNumber = svdHit2D->getSensorID().getLayerNumber();
        const auto& currentHits = hitPatternVXD.getSVDLayer(layerNumber);
        hitPatternVXD.setSVDLayer(layerNumber, currentHits.first + 1, currentHits.second + 1);
      } else if (svdHit) {
        const int layerNumber = svdHit->getSensorID().getLayerNumber();
        const auto& currentHits = hitPatternVXD.getSVDLayer(layerNumber);

        if (svdHit->isU())
          hitPatternVXD.setSVDLayer(layerNumber, currentHits.first + 1, currentHits.second);
        else
          hitPatternVXD.setSVDLayer(layerNumber, currentHits.first , currentHits.second + 1);
      }

    }
  }

  if (nNotFittedVXDhits > 0) {
    B2DEBUG(100, " No KalmanFitterInfo associated to some TrackPoints with VXD hits, not filling the HitPatternVXD");
    B2DEBUG(100, nNotFittedVXDhits << " had no FitterInfo");
  }
  return hitPatternVXD.getInteger();
}

///// copy from TrackBuilder::getHitPatternCDCInitializer
uint64_t V0Fitter::getHitPatternCDCInitializer(const genfit::Track& track) const
{
  HitPatternCDC hitPatternCDC;

  int nCDChits = 0;
  int nNotFittedCDChits = 0;

  const auto& hitPointsWithMeasurements = track.getPointsWithMeasurement();

  for (const auto& trackPoint : hitPointsWithMeasurements) {

    for (size_t measurementId = 0; measurementId < trackPoint->getNumRawMeasurements(); measurementId++) {

      genfit::AbsMeasurement* absMeas = trackPoint->getRawMeasurement(measurementId);
      genfit::KalmanFitterInfo* kalmanInfo = trackPoint->getKalmanFitterInfo();

      if (kalmanInfo) {
        const double weight = kalmanInfo->getWeights().at(measurementId);
        if (weight == 0)
          continue;
      } else {
        ++nNotFittedCDChits;
        continue;
      }

      CDCRecoHit* cdcHit = dynamic_cast<CDCRecoHit*>(absMeas);

      if (cdcHit) {
        WireID wire = cdcHit->getWireID();
        hitPatternCDC.setLayer(wire.getICLayer());
        nCDChits++;
      }
    }

  }
  if (nNotFittedCDChits > 0) {
    B2DEBUG(100, " No KalmanFitterInfo associated to some TrackPoints with CDC hits, not filling the HitPatternCDC");
    B2DEBUG(100, nNotFittedCDChits << " out of " << nCDChits << " had no FitterInfo");
  }
  hitPatternCDC.setNHits(nCDChits);

  return hitPatternCDC.getInteger();

}
