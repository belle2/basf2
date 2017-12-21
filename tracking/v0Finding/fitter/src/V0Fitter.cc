#include <tracking/v0Finding/fitter/V0Fitter.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/v0Finding/dataobjects/VertexVector.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/GFRaveVertexFactory.h>
#include <genfit/GFRaveVertex.h>
#include <genfit/Track.h>
#include <genfit/FieldManager.h>
#include "genfit/MaterialEffects.h"
#include <genfit/Exception.h>

#include <framework/utilities/IOIntercept.h>

using namespace Belle2;

V0Fitter::V0Fitter(const std::string& trackFitResultsName, const std::string& v0sName,
                   const std::string& v0ValidationVerticesName, const std::string& recoTracksName,
                   bool enableValidation)
  : m_validation(enableValidation), m_recoTracksName(recoTracksName)
{
  m_trackFitResults.isRequired(trackFitResultsName);
  m_v0s.registerInDataStore(v0sName, DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  //Relation to RecoTracks from Tracks is already tested at the module level.

  if (m_validation) {
    B2DEBUG(300, "Register DataStore for validation.");
    m_validationV0s.registerInDataStore(v0ValidationVerticesName);
  }

  B2ASSERT(genfit::MaterialEffects::getInstance()->isInitialized(),
           "Material effects not set up.  Please use SetupGenfitExtrapolationModule.");
  B2ASSERT(genfit::FieldManager::getInstance()->isInitialized(),
           "Magnetic field not set up.  Please use SetupGenfitExtrapolationModule.");
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


bool V0Fitter::fitVertex(genfit::Track& trackPlus, genfit::Track& trackMinus, genfit::GFRaveVertex& vertex)
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
  TrackFitResult* v0TrackFitResult
    = m_trackFitResults.appendNew(msop.getPos(), msop.getMom(),
                                  msop.get6DCov(), msop.getCharge(),
                                  trackHypothesis,
                                  track.getFitStatus()->getPVal(),
                                  Bz, 0, 0);
  return v0TrackFitResult;
}

std::pair<Const::ParticleType, Const::ParticleType> V0Fitter::getTrackHypotheses(const Const::ParticleType& v0Hypothesis)
{
  if (v0Hypothesis == Const::Kshort) {
    return std::make_pair(Const::pion, Const::pion);
  } else if (v0Hypothesis == Const::photon) {
    return std::make_pair(Const::electron, Const::electron);
  } else if (v0Hypothesis == Const::Lambda) {
    return std::make_pair(Const::proton, Const::pion);
  } else if (v0Hypothesis == Const::antiLambda) {
    return std::make_pair(Const::pion, Const::proton);
  } else {
    B2FATAL("Given V0Hypothesis not available.");
    return std::make_pair(Const::invalidParticle, Const::invalidParticle);
  }

}

bool V0Fitter::fitAndStore(const Track* trackPlus, const Track* trackMinus,
                           const Const::ParticleType& v0Hypothesis)
{
  const auto trackHypotheses = getTrackHypotheses(v0Hypothesis);

  RecoTrack* recoTrackPlus = trackPlus->getRelated<RecoTrack>(m_recoTracksName);
  if (not recoTrackPlus) {
    B2ERROR("No RecoTrack for Belle2::Track");
    return false;
  }
  genfit::Track gfTrackPlus = RecoTrackGenfitAccess::getGenfitTrack(*recoTrackPlus);

  int pdgTrackPlus = trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getParticleType().getPDGCode();

  genfit::AbsTrackRep* plusRepresentation = TrackFitter::getTrackRepresentationForPDG(pdgTrackPlus, *recoTrackPlus);
  if (not recoTrackPlus->wasFitSuccessful(plusRepresentation)) {
    B2ERROR("Default track hypothesis not available. Should never happen, but I can continue savely anyway.");
    return false;
  }

  RecoTrack* recoTrackMinus = trackMinus->getRelated<RecoTrack>(m_recoTracksName);
  if (not recoTrackMinus) {
    B2ERROR("No RecoTrack for Belle2::Track");
    return false;
  }

  genfit::Track gfTrackMinus = RecoTrackGenfitAccess::getGenfitTrack(*recoTrackMinus);

  int pdgTrackMinus = trackMinus->getTrackFitResultWithClosestMass(trackHypotheses.second)->getParticleType().getPDGCode();

  genfit::AbsTrackRep* minusRepresentation = TrackFitter::getTrackRepresentationForPDG(pdgTrackMinus, *recoTrackMinus);
  if (not recoTrackMinus->wasFitSuccessful(minusRepresentation)) {
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

  genfit::MeasuredStateOnPlane stPlus = recoTrackPlus->getMeasuredStateOnPlaneFromFirstHit(plusRepresentation);
  genfit::MeasuredStateOnPlane stMinus = recoTrackMinus->getMeasuredStateOnPlaneFromFirstHit(minusRepresentation);

  if (rejectCandidate(stPlus, stMinus)) {
    return false;
  }

  genfit::GFRaveVertex vert;
  if (not fitVertex(gfTrackPlus, gfTrackMinus, vert)) {
    return false;
  }

  // Update the tracks (genfit::MeasuredStateOnPlane) after vertex
  stPlus = gfTrackPlus.getFittedState();
  stMinus = gfTrackMinus.getFittedState();

  const genfit::GFRaveTrackParameters* tr0 = vert.getParameters(0);
  const genfit::GFRaveTrackParameters* tr1 = vert.getParameters(1);

  const TVector3& posVert(vert.getPos());
  TLorentzVector lv0, lv1;

  // Reconstruct invariant mass.
  lv0.SetVectM(tr0->getMom(), trackHypotheses.first.getMass());
  lv1.SetVectM(tr1->getMom(), trackHypotheses.second.getMass());

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

  // To build the trackFitResult  use the magnetic field at the origin;
  // the helix is extrapolated to the IP in a constant magnetic field and material effects are neglected
  // so that the vertexing tool executed on the MDST object will find again this vertex position
  const TVector3 origin(0, 0, 0);
  const double Bz = getBzAtVertex(origin);

  TrackFitResult* tfrPlusVtx = buildTrackFitResult(gfTrackPlus, stPlus, Bz, trackHypotheses.first);
  TrackFitResult* tfrMinusVtx = buildTrackFitResult(gfTrackMinus, stMinus, Bz, trackHypotheses.second);

  B2DEBUG(100, "Creating new V0.");
  m_v0s.appendNew(std::make_pair(trackPlus, tfrPlusVtx),
                  std::make_pair(trackMinus, tfrMinusVtx));

  if (m_validation) {
    B2DEBUG(300, "Create StoreArray and Output for validation.");
    m_validationV0s.appendNew(
      std::make_pair(trackPlus, tfrPlusVtx),
      std::make_pair(trackMinus, tfrMinusVtx),
      vert.getPos(),
      vert.getCov(),
      (lv0 + lv1).P(),
      (lv0 + lv1).M(),
      vert.getChi2()
    );

  }
  return true;
}
