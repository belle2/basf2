#include <tracking/v0Finding/fitter/V0Fitter.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/v0Finding/dataobjects/VertexVector.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/GFRaveVertexFactory.h>
#include <genfit/GFRaveVertex.h>
#include <genfit/Track.h>
#include <genfit/FieldManager.h>
#include <genfit/Exception.h>

#include <TVector3.h>

using namespace Belle2;

V0Fitter::V0Fitter(const std::string& trackFitResultColName, const std::string& v0ColName,
                   const std::string& v0ValidationVertexColName, const std::string& gfTrackColName)
  : m_validation(false), m_RecoTrackColName(gfTrackColName)
{
  m_trackFitResults = StoreArray<TrackFitResult>(trackFitResultColName);
  m_v0s = StoreArray<V0>(v0ColName);
  m_validationV0s = StoreArray<V0ValidationVertex>(v0ValidationVertexColName);
}

void V0Fitter::initializeCuts(double beamPipeRadius,
                              double vertexChi2CutInside,
                              double massWindowKshortInside,
                              double vertexChi2CutOutside)
{
  m_beamPipeRadius = beamPipeRadius;
  m_vertexChi2CutInside = vertexChi2CutInside;
  m_massWindowKshortInside = massWindowKshortInside;
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
    B2ERROR("Wrong number of tracks in vertex.");
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
    B2WARNING("Could not extrapolate track to vertex.");
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
                           const Const::ParticleType& v0Hypothesis)  // TODO: Give Particle Types
{
  const auto trackHypotheses = getTrackHypotheses(v0Hypothesis);

  RecoTrack* recoTrackPlus = trackPlus->getRelated<RecoTrack>(m_RecoTrackColName);
  if (not recoTrackPlus) {
    B2ERROR("No RecoTrack for Belle2::Track");
    return false;
  }
  genfit::Track& gfTrackPlus = RecoTrackGenfitAccess::getGenfitTrack(*recoTrackPlus);

  RecoTrack* recoTrackMinus = trackMinus->getRelated<RecoTrack>(m_RecoTrackColName);
  if (not recoTrackMinus) {
    B2ERROR("No RecoTrack for Belle2::Track");
    return false;
  }
  genfit::Track& gfTrackMinus = RecoTrackGenfitAccess::getGenfitTrack(*recoTrackMinus);

  const genfit::MeasuredStateOnPlane* stPlusPtr = nullptr;
  try {
    stPlusPtr = &gfTrackPlus.getFittedState();
  } catch (genfit::Exception) {
    B2DEBUG(100, "No TrackPoint with fitterInfo for the given rep.");
    return false;
  }

  const genfit::MeasuredStateOnPlane* stMinusPtr = nullptr;
  try {
    stMinusPtr = &gfTrackMinus.getFittedState();
  } catch (genfit::Exception) {
    B2DEBUG(100, "No TrackPoint with fitterInfo for the given rep.");
    return false;
  }

  if (not stPlusPtr or not stMinusPtr) {
    return false;
  }

  genfit::MeasuredStateOnPlane stPlus = *stPlusPtr;
  genfit::MeasuredStateOnPlane stMinus = *stMinusPtr;

  if (rejectCandidate(stPlus, stMinus)) {
    return false;
  }

  genfit::GFRaveVertex vert;
  if (not fitVertex(gfTrackPlus, gfTrackMinus, vert)) {
    return false;
  }

  const genfit::GFRaveTrackParameters* tr0 = vert.getParameters(0);
  const genfit::GFRaveTrackParameters* tr1 = vert.getParameters(1);

  const TVector3& posVert(vert.getPos());
  TLorentzVector lv0, lv1;

  // Reconstruct invariant mass.
  lv0.SetVectM(tr0->getMom(), trackHypotheses.first.getMass());
  lv1.SetVectM(tr1->getMom(), trackHypotheses.second.getMass());

  // Apply cuts.  We have one set of cuts inside the beam pipe,
  // the other outside.
  if (posVert.Perp() < m_beamPipeRadius) {
    if (v0Hypothesis == Const::photon) {
      return false;  // No converions inside beampipe
    }

    if (vert.getChi2() > m_vertexChi2CutInside) {
      B2DEBUG(200, "Vertex inside beam pipe, chi^2 too large.");
      return false;
    }

    const double mReco = (lv0 + lv1).M();
    if (v0Hypothesis == Const::Kshort and fabs(mReco - Const::K0Mass) > m_massWindowKshortInside * Unit::MeV) {
      B2DEBUG(200, "Vertex inside beam pipe, outside Kshort mass window.");
      return false;
    }
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

  const double Bz = getBzAtVertex(posVert);

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
