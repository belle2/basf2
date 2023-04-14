/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/v0Finding/fitter/NewV0Fitter.h>

#include <framework/logging/Logger.h>
#include <framework/geometry/BFieldManager.h>
#include <tracking/v0Finding/dataobjects/VertexVector.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/trackFitting/trackBuilder/factories/TrackBuilder.h>

#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/HitPatternCDC.h>

#include <genfit/GFRaveVertexFactory.h>
#include <genfit/FieldManager.h>
#include <genfit/MaterialEffects.h>

#include <framework/utilities/IOIntercept.h>

using namespace Belle2;

NewV0Fitter::NewV0Fitter(const std::string& trackFitResultsName, const std::string& v0sName,
                         const std::string& v0ValidationVerticesName, const std::string& recoTracksName,
                         const std::string& copiedRecoTracksName, bool enableValidation)
  : m_recoTracksName(recoTracksName), m_validation(enableValidation)
{
  m_recoTracks.isRequired(m_recoTracksName);
  m_trackFitResults.isRequired(trackFitResultsName);

  m_v0s.registerInDataStore(v0sName, DataStore::c_WriteOut | DataStore::c_ErrorIfAlreadyRegistered);
  m_copiedRecoTracks.registerInDataStore(copiedRecoTracksName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_copiedRecoTracks);
  m_copiedRecoTracks.registerRelationTo(m_recoTracks);

  if (m_validation) {
    m_validationV0s.registerInDataStore(v0ValidationVerticesName, DataStore::c_ErrorIfAlreadyRegistered);
    m_v0s.registerRelationTo(m_validationV0s);
  }

  B2ASSERT("V0Fitter: material effects not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::MaterialEffects::getInstance()->isInitialized());
  B2ASSERT("V0Fitter: magnetic field not set up.  Please use SetupGenfitExtrapolationModule.",
           genfit::FieldManager::getInstance()->isInitialized());
}


void NewV0Fitter::initializeCuts(double beamPipeRadius,
                                 double vertexChi2Cut,
                                 const std::tuple<double, double>& invMassRangeKshort,
                                 const std::tuple<double, double>& invMassRangeLambda,
                                 const std::tuple<double, double>& invMassRangePhoton)
{
  m_beamPipeRadius = beamPipeRadius;
  m_vertexChi2Cut = vertexChi2Cut;
  m_invMassCuts[22] = std::make_pair(std::get<0>(invMassRangePhoton), std::get<1>(invMassRangePhoton));
  m_invMassCuts[310] = std::make_pair(std::get<0>(invMassRangeKshort), std::get<1>(invMassRangeKshort));
  m_invMassCuts[3122] = std::make_pair(std::get<0>(invMassRangeLambda), std::get<1>(invMassRangeLambda));
}


std::pair<Const::ParticleType, Const::ParticleType> NewV0Fitter::getTrackHypotheses(const Const::ParticleType& v0Hypothesis)
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
  B2FATAL("V0Fitter: given V0 hypothesis not available.");
  return std::make_pair(Const::invalidParticle, Const::invalidParticle); // return something to avoid triggering cppcheck
}


bool NewV0Fitter::fitAndStore(const Track* trackPlus, const Track* trackMinus, const Const::ParticleType& v0Hypothesis,
                              bool& isForceStored, bool& isHitRemoved)
{
  /// Initialize status flags and result storage
  isForceStored = false;
  isHitRemoved = false;
  m_trkPlus.recoTrack = 0;
  m_trkMinus.recoTrack = 0;

  /// Get related recoTracks
  const RecoTrack* recoTrackPlus  =  trackPlus->getRelated<RecoTrack>(m_recoTracksName);
  if (not recoTrackPlus) return false;
  const RecoTrack* recoTrackMinus = trackMinus->getRelated<RecoTrack>(m_recoTracksName);
  if (not recoTrackMinus) return false;

  /// PDG codes actually used in track fitting
  auto trackHypotheses = getTrackHypotheses(v0Hypothesis);
  auto ptypeTrackPlus  = trackPlus->getTrackFitResultWithClosestMass(trackHypotheses.first)->getParticleType();
  auto ptypeTrackMinus = trackMinus->getTrackFitResultWithClosestMass(trackHypotheses.second)->getParticleType();
  int pdgTrackPlus  = std::abs(ptypeTrackPlus.getPDGCode());
  int pdgTrackMinus = std::abs(ptypeTrackMinus.getPDGCode());

  /// initial vertex fit
  int status = vertexFit(recoTrackPlus, recoTrackMinus, pdgTrackPlus, pdgTrackMinus, v0Hypothesis);
  if (status < 0) return false;

  if (m_fitterMode > 0) {
    /// try to refit tracks by removing inner hits and refit the vertex
    const RecoTrack* origRecoTrackPlus = recoTrackPlus;
    const RecoTrack* origRecoTrackMinus = recoTrackMinus;
    int counter = 0;
    while (status != 0 and counter < 5) {
      counter++;
      const RecoTrack* trkPlus = recoTrackPlus;
      if (status & 0x1) {
        trkPlus = removeHitsAndRefit(origRecoTrackPlus, recoTrackPlus, ptypeTrackPlus);
        if (not trkPlus) return false;
        isHitRemoved = true;
      }
      const RecoTrack* trkMinus = recoTrackMinus;
      if (status & 0x2) {
        trkMinus = removeHitsAndRefit(origRecoTrackMinus, recoTrackMinus, ptypeTrackMinus);
        if (not trkMinus) return false;
        isHitRemoved = true;
      }
      if (trkPlus == recoTrackPlus and trkMinus == recoTrackMinus) break; // vertex fit already done, so exit the loop
      status = vertexFit(trkPlus, trkMinus, pdgTrackPlus, pdgTrackMinus, v0Hypothesis);
      if (status < 0) break; // save the results of the last successful iteration
      recoTrackPlus = trkPlus;
      recoTrackMinus = trkMinus;
    }

    isForceStored = (status != 0);
  }

  /// save the results
  int sharedCluster = isInnermostClusterShared(recoTrackPlus, recoTrackMinus);
  const auto* fitPlus = saveTrackFitResult(m_trkPlus, sharedCluster);
  if (not fitPlus) return false;
  const auto* fitMinus = saveTrackFitResult(m_trkMinus, sharedCluster);
  if (not fitMinus) return false;

  auto* v0 = m_v0s.appendNew(std::make_pair(trackPlus, fitPlus),
                             std::make_pair(trackMinus, fitMinus));

  if (m_validation) {
    auto* validationV0 = m_validationV0s.appendNew(std::make_pair(trackPlus, fitPlus),
                                                   std::make_pair(trackMinus, fitMinus),
                                                   ROOT::Math::XYZVector(m_fittedVertex.getPos()),
                                                   m_fittedVertex.getCov(),
                                                   m_momentum, m_invMass,
                                                   m_fittedVertex.getChi2());
    v0->addRelationTo(validationV0);
  }

  return true;
}


int NewV0Fitter::vertexFit(const RecoTrack* recoTrackPlus, const RecoTrack* recoTrackMinus,
                           int pdgTrackPlus, int pdgTrackMinus, const Const::ParticleType& v0Hypothesis)
{

  // get track representations for given PDG codes and check their existance

  const auto* plusRepresentation = getTrackRepresentation(recoTrackPlus, pdgTrackPlus);
  if (not plusRepresentation) return c_NoTrackRepresentation;

  const auto* minusRepresentation = getTrackRepresentation(recoTrackMinus, pdgTrackMinus);
  if (not minusRepresentation) return c_NoTrackRepresentation;

  // make copies of genfit tracks which will be passed to vertex fit and set the cardinal representations

  auto gfTrackPlus = recoTrackPlus->getGenfitTrack();   // a copy of
  if (not setCardinalRep(gfTrackPlus, pdgTrackPlus)) return c_NoTrackRepresentation;

  auto gfTrackMinus = recoTrackMinus->getGenfitTrack(); // a copy of
  if (not setCardinalRep(gfTrackMinus, pdgTrackMinus)) return c_NoTrackRepresentation;

  // fit vertex

  genfit::GFRaveVertex vert;
  if (not fitGFRaveVertex(gfTrackPlus, gfTrackMinus, vert)) return c_VertexFitFailed;
  auto vertexPos = ROOT::Math::XYZVector(vert.getPos());

  // apply cuts on the vertex

  if (vertexPos.Rho() < m_beamPipeRadius) return c_NotSelected;
  if (vert.getChi2() > m_vertexChi2Cut) return c_NotSelected;

  // apply cut on the invariant mass

  const auto& p1 = vert.getParameters(0)->getMom();
  const auto& p2 = vert.getParameters(1)->getMom();
  auto trackHypotheses = getTrackHypotheses(v0Hypothesis);
  double mass1 = trackHypotheses.first.getMass();
  double mass2 = trackHypotheses.second.getMass();
  ROOT::Math::PxPyPzMVector lv1(p1.X(), p1.Y(), p1.Z(), mass1);
  ROOT::Math::PxPyPzMVector lv2(p2.X(), p2.Y(), p2.Z(), mass2);
  double invMass = (lv1 + lv2).M();
  int pdg = abs(v0Hypothesis.getPDGCode());
  const auto& cuts = m_invMassCuts[pdg];
  if (invMass < cuts.first or invMass > cuts.second) return c_NotSelected;

  // extrapolate tracks to fitted vertex; the return status, if positive, indicates whether there are inner hits

  auto statePlus = recoTrackPlus->getMeasuredStateOnPlaneFromFirstHit(plusRepresentation); // a copy of
  auto stateMinus = recoTrackMinus->getMeasuredStateOnPlaneFromFirstHit(minusRepresentation); // a copy of
  int status = extrapolateToVertex(statePlus, stateMinus, vert);
  if (status < 0) return c_ExtrapolationFailed;

  // save fitted vertex and tracks

  m_fittedVertex = vert;
  m_momentum = (lv1 + lv2).P();
  m_invMass = invMass;
  m_trkPlus.set(recoTrackPlus, trackHypotheses.first, statePlus, plusRepresentation);
  m_trkMinus.set(recoTrackMinus, trackHypotheses.second, stateMinus, minusRepresentation);

  return status;
}


const genfit::AbsTrackRep* NewV0Fitter::getTrackRepresentation(const RecoTrack* recoTrack, int pdgCode)
{
  for (const auto* rep : recoTrack->getGenfitTrack().getTrackReps()) {
    // Check if the track representation is a RKTrackRep.
    const auto* rkTrackRepresenation = dynamic_cast<const genfit::RKTrackRep*>(rep);
    if (rkTrackRepresenation) {
      if (std::abs(rkTrackRepresenation->getPDG()) == pdgCode and recoTrack->wasFitSuccessful(rep)) return rep;
    }
  }

  B2ERROR("V0Fitter: track hypothesis with closest mass not available. Should never happen!");
  return 0;
}


bool NewV0Fitter::setCardinalRep(genfit::Track& gfTrack, int pdg)
{
  const auto& reps = gfTrack.getTrackReps();
  for (unsigned id = 0; id < reps.size(); id++) {
    if (abs(reps[id]->getPDG()) == pdg) {
      gfTrack.setCardinalRep(id);
      return true;
    }
  }

  B2ERROR("V0Fitter: cannot set cardinal representation for PDG = " << pdg);
  return false;
}


bool NewV0Fitter::fitGFRaveVertex(genfit::Track& trackPlus, genfit::Track& trackMinus, genfit::GFRaveVertex& vertex)
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
    B2ERROR("V0Fitter: exception during vertex fit.");
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


int NewV0Fitter::extrapolateToVertex(genfit::MeasuredStateOnPlane& statePlus, genfit::MeasuredStateOnPlane& stateMinus,
                                     const genfit::GFRaveVertex& vertex)
{
  try {
    /// extrapolate the first (innermost) hit to the V0 vertex position
    /// the value will be positive (negative) if the direction of the extrapolation is (counter)momentum-wise
    double extralengthPlus  =  statePlus.extrapolateToPoint(vertex.getPos());
    double extralengthMinus =  stateMinus.extrapolateToPoint(vertex.getPos());
    unsigned status = 0;
    if (extralengthPlus  > 0) status |= 0x1; ///  plus track has hits inside the V0 vertex.
    if (extralengthMinus > 0) status |= 0x2; /// minus track has hits inside the V0 vertex.
    return status;
  } catch (...) {
    /// This shouldn't ever happen, but I can see the extrapolation
    /// code trying several windings before giving up, so this happens occasionally.
    /// Something more stable would perhaps be desirable.
    B2DEBUG(22, "Could not extrapolate track to vertex.");
    return c_ExtrapolationFailed;
  }
}


const RecoTrack* NewV0Fitter::removeHitsAndRefit(const RecoTrack* origRecoTrack, const RecoTrack* lastRecoTrack,
                                                 const Const::ParticleType& ptype)
{
  // make a copy of useInFit flags
  std::vector<bool> useInFit;
  const auto& recoHitInformations = origRecoTrack->getRecoHitInformations(true); // true to get sorted hits info
  for (const auto& hitInfo : recoHitInformations) useInFit.push_back(hitInfo->useInFit());

  // get track representation for a given particle
  const auto* rep = getTrackRepresentation(origRecoTrack, abs(ptype.getPDGCode()));
  if (not rep) return lastRecoTrack;

  // remove inner hits by setting useInFit flags to false
  int removedHits = 0;
  unsigned firstHit = 0;
  for (unsigned i = 0; i < recoHitInformations.size(); i++) {
    const auto& hitInfo = recoHitInformations[i];
    if (not hitInfo->useInFit()) continue;
    try {
      auto state = origRecoTrack->getMeasuredStateOnPlaneFromRecoHit(hitInfo, rep); // a copy of
      double extraLength = state.extrapolateToPoint(m_fittedVertex.getPos());
      if (extraLength > 0) {
        useInFit[i] = false;
        removedHits++;
      } else {
        firstHit = i;
        break;
      }
    } catch (NoTrackFitResult()) {
      B2WARNING("V0Fitter exception: no FitterInfo assigned for TrackPoint created from this RecoHit.");
      useInFit[i] = false;
      removedHits++;
      continue;
    } catch (...) {
      /// This shouldn't ever happen, but I can see the extrapolation
      /// code trying several windings before giving up, so this happens occasionally.
      /// Something more stable would perhaps be desirable.
      B2DEBUG(22, "Could not extrapolate track to vertex when removing inner hits.");
      return lastRecoTrack;
    }
  }

  if (m_fitterMode == 2) {
    // remove SVD hits if there is only one or two left just after the vertex
    std::vector<unsigned> svdIndex;
    for (unsigned i = 0; i < useInFit.size(); i++) {
      if (not useInFit[i]) continue;
      const auto& hitInfo = recoHitInformations[i];
      if (hitInfo->getTrackingDetector() == RecoHitInformation::c_SVD) svdIndex.push_back(i);
      else break;
    }
    if (not svdIndex.empty() and svdIndex.size() < 3) {
      for (unsigned i : svdIndex) {
        useInFit[i] = false;
        removedHits++;
      }
    }
  }

  if (removedHits == 0) return origRecoTrack;  // in this case track doesn't need to be refitted

  // count remaining hits and return if there is no hope for the fit to succeed
  int nHits = 0;
  for (auto x : useInFit) if (x) nHits++;
  if (nHits < 5) return lastRecoTrack;

  // make a copy of recoTrack
  const auto& state = origRecoTrack->getMeasuredStateOnPlaneFromRecoHit(recoHitInformations[firstHit], rep);
  auto* recoTrack_copy = copyRecoTrack(origRecoTrack, state);

  // set useInFit flags in a copy of recoTrack
  const auto& recoHitInfos = recoTrack_copy->getRecoHitInformations(true); // true to get sorted hits info
  if (recoHitInfos.size() != useInFit.size()) {
    B2ERROR("V0Fitter: copied recoTrack has different number of hits than the original one");
    return lastRecoTrack;
  }
  for (unsigned i = 0; i < recoHitInfos.size(); i++) recoHitInfos[i]->setUseInFit(useInFit[i]);

  // fit a copy of recoTrack
  TrackFitter fitter;
  bool ok = fitter.fit(*recoTrack_copy, ptype);
  if (not ok) return lastRecoTrack;

  return recoTrack_copy;
}


RecoTrack* NewV0Fitter::copyRecoTrack(const RecoTrack* origRecoTrack, const genfit::MeasuredStateOnPlane& state)
{
  RecoTrack* newRecoTrack = origRecoTrack->copyToStoreArrayUsing(m_copiedRecoTracks,
                            ROOT::Math::XYZVector(state.getPos()),
                            ROOT::Math::XYZVector(state.getMom()),
                            state.getCharge(),
                            state.get6DCov(), state.getTime());
  newRecoTrack->addHitsFromRecoTrack(origRecoTrack);
  newRecoTrack->addRelationTo(origRecoTrack);
  return newRecoTrack;
}


int NewV0Fitter::isInnermostClusterShared(const RecoTrack* recoTrackPlus, const RecoTrack* recoTrackMinus)
{
  const auto& recoHitInformationsPlus = recoTrackPlus->getRecoHitInformations(true); // true to get sorted hits info
  std::vector<RecoHitInformation*> innerHitsPlus;
  for (const auto& hitInfo : recoHitInformationsPlus) {
    if (hitInfo->useInFit()) innerHitsPlus.push_back(hitInfo);
    if (innerHitsPlus.size() == 2) break;
  }
  if (innerHitsPlus.empty()) return 0;

  const auto& recoHitInformationsMinus = recoTrackMinus->getRecoHitInformations(true); // true to get sorted hits info
  std::vector<RecoHitInformation*> innerHitsMinus;
  for (const auto& hitInfo : recoHitInformationsMinus) {
    if (hitInfo->useInFit()) innerHitsMinus.push_back(hitInfo);
    if (innerHitsMinus.size() == 2) break;
  }
  if (innerHitsMinus.empty()) return 0;

  if (innerHitsPlus.front()->getTrackingDetector() != innerHitsMinus.front()->getTrackingDetector()) return 0;

  if (innerHitsPlus.front()->getTrackingDetector() == RecoHitInformation::c_PXD) {
    const auto* clusterPlus = innerHitsPlus.front()->getRelatedTo<PXDCluster>();
    const auto* clusterMinus = innerHitsMinus.front()->getRelatedTo<PXDCluster>();
    if (clusterPlus and clusterPlus == clusterMinus) return 0x03;
    return 0;
  }

  int flag = 0;
  VxdID sensorID = 0;
  if (innerHitsPlus.front()->getTrackingDetector() == RecoHitInformation::c_SVD) {
    const auto* clusterPlus = innerHitsPlus.front()->getRelatedTo<SVDCluster>();
    const auto* clusterMinus = innerHitsMinus.front()->getRelatedTo<SVDCluster>();
    if (clusterPlus and clusterPlus == clusterMinus) {
      sensorID = clusterPlus->getSensorID();
      if (clusterPlus->isUCluster()) flag = 0x01;
      else flag = 0x02;
    }
  }

  if (innerHitsPlus.size() != 2 or innerHitsMinus.size() != 2) return flag;

  if (innerHitsPlus.back()->getTrackingDetector() == RecoHitInformation::c_SVD) {
    const auto* clusterPlus = innerHitsPlus.back()->getRelatedTo<SVDCluster>();
    const auto* clusterMinus = innerHitsMinus.back()->getRelatedTo<SVDCluster>();
    if (clusterPlus and clusterPlus == clusterMinus and clusterPlus->getSensorID() == sensorID) {
      if (clusterPlus->isUCluster()) flag |= 0x01;
      else flag |= 0x02;
    }
  }

  return flag;
}


const TrackFitResult* NewV0Fitter::saveTrackFitResult(const FittedTrack& trk, int sharedInnermostCluster)
{

  const auto* recoTrack = trk.recoTrack;
  if (not recoTrack) {
    B2ERROR("V0Fitter: bug in saving track fit result, recoTrack is nullptr");
    return 0;
  }

  auto hitPatternCDC = TrackBuilder::getHitPatternCDCInitializer(*recoTrack);
  auto hitPatternVXD = TrackBuilder::getHitPatternVXDInitializer(*recoTrack);
  if (sharedInnermostCluster > 0) {
    auto pattern = HitPatternVXD(hitPatternVXD);
    pattern.setInnermostHitShareStatus(sharedInnermostCluster);
    hitPatternVXD = pattern.getInteger();
  }
  double Bz = BFieldManager::getFieldInTesla({0, 0, 0}).Z();
  const auto& state = trk.state;

  auto* fit = m_trackFitResults.appendNew(ROOT::Math::XYZVector(state.getPos()), ROOT::Math::XYZVector(state.getMom()),
                                          state.get6DCov(), state.getCharge(), trk.ptype, trk.pValue,
                                          Bz, hitPatternCDC, hitPatternVXD, trk.Ndf);
  return fit;
}
