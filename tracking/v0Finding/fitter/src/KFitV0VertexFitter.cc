/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include <tracking/v0Finding/fitter/KFitV0VertexFitter.h>

#include <framework/geometry/BFieldManager.h>
#include <framework/logging/Logger.h>
#include <framework/particledb/EvtGenDatabasePDG.h>

#include <analysis/utility/ROOTToCLHEP.h>
#include <analysis/utility/CLHEPToROOT.h>
#include <analysis/VertexFitting/KFit/KFitError.h>
#include <analysis/VertexFitting/KFit/VertexFitKFit.h>

#include <genfit/Track.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/FitStatus.h>
#include <genfit/GFRaveVertex.h>
#include <genfit/GFRaveTrackParameters.h>

#include <TParticlePDG.h>

#include <cmath>

using namespace Belle2;

namespace {

  CLHEP::HepSymMatrix makeCov7x7(
    const TMatrixDSym& cov6,
    const CLHEP::HepLorentzVector& p4)
  {
    // ordering expected by KFit:
    // (px,py,pz,E,x,y,z)
    // cov6 from getPosMomCov has ordering:
    // (x,y,z,px,py,pz)
    // so we need to reindex: KFit 0,1,2 (px,py,pz) <- cov6 3,4,5
    //                        KFit 4,5,6 (x,y,z)    <- cov6 0,1,2
    constexpr int toCov6[7] = {3, 4, 5, -1, 0, 1, 2}; // -1 = E, handled separately
    constexpr int pIdx[3] = {3, 4, 5}; // momentum indices in cov6

    CLHEP::HepSymMatrix cov7(7, 0);

    // copy 6x6 block (lower triangle only), skipping E (index 3)
    for (int i = 0; i < 7; ++i) {
      if (i == 3) continue;
      for (int j = i; j < 7; ++j) {
        if (j == 3) continue;
        cov7[j][i] = cov6(toCov6[j], toCov6[i]);
      }
    }

    // calculate E-related derivatives
    const double E = p4.e();
    const double dEdp[3] = {
      p4.px() / E,
      p4.py() / E,
      p4.pz() / E
    };

    // cov(E, i) for all i != E (lower triangle only)
    for (int i = 0; i < 7; ++i) {
      if (i == 3) continue;
      double covEi = 0.0;
      for (int k = 0; k < 3; ++k) {
        covEi += cov6(toCov6[i], pIdx[k]) * dEdp[k];
      }
      // index 3 is E; since i != 3, we always have i < 3 or i > 3
      if (i < 3) {
        cov7[3][i] = covEi; // lower triangle: row > col
      } else {
        cov7[i][3] = covEi; // lower triangle: row > col
      }
    }

    // cov(E,E)
    double covEE = 0.0;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        covEE += cov6(pIdx[i], pIdx[j]) * dEdp[i] * dEdp[j];
      }
    }
    cov7[3][3] = covEE;

    return cov7;
  }

  double getMagneticField()
  {
    const double bZ = BFieldManager::getFieldInTesla({0, 0, 0}).Z();
    // KFit handles a null field analytically, as a straight-line fit (see VertexFitKFit::makeCoreMatrix),
    // while a residual field would take the helix branch and make it ill-conditioned:
    // snap to zero what cannot bend a track (1 uT bends a 100 MeV/c track by less than 1 um per meter of flight).
    return std::abs(bZ) > 1e-6 ? bZ : 0.;
  }

}

bool KFitV0VertexFitter::fit(genfit::Track& trackPlus, genfit::Track& trackMinus, const int pdgTrackPlus,
                             const int pdgTrackMinus, genfit::GFRaveVertex& vertex)
{
  analysis::VertexFitKFit vertexFit;
  // KFit defaults to KFitConst::kDefaultMagneticField:
  // use instead the same magnetic field used elsewhere in the V0Fitter
  vertexFit.setMagneticField(getMagneticField());

  EvtGenDatabasePDG* pdgDB = EvtGenDatabasePDG::Instance();

  auto addTrackToFit =
  [&](const genfit::Track & track, const int pdg) {
    const TParticlePDG* particle = pdgDB->GetParticle(pdg);
    if (not particle) {
      B2ERROR("Unknown PDG code of the daughter hypothesis." << LogVar("PDG code", pdg));
      return false;
    }
    TVector3 pos;
    TVector3 mom;
    // KFit needs the 7x7 cov. matrix, not the 6x6 one:
    // in the 6x6 one, correlations with E are missing.
    TMatrixDSym cov6;
    const genfit::MeasuredStateOnPlane state = track.getFittedState();
    state.getPosMomCov(pos, mom, cov6);
    // the mass is used only to complete the energy: the vertex fit itself drops it
    const double mass = particle->Mass();
    const CLHEP::HepLorentzVector clhepMom(mom.X(), mom.Y(), mom.Z(), std::sqrt(mom.Mag2() + mass * mass));
    const HepGeom::Point3D<double> clhepPos(pos.X(), pos.Y(), pos.Z());
    // This is now good for KFit
    const CLHEP::HepSymMatrix clhepCov7 = makeCov7x7(cov6, clhepMom);
    const int charge = track.getFitStatus()->getCharge();

    vertexFit.addTrack(clhepMom, clhepPos, clhepCov7, charge);
    return true;
  };

  try {
    if (not addTrackToFit(trackPlus, pdgTrackPlus)) return false;
    if (not addTrackToFit(trackMinus, pdgTrackMinus)) return false;
  } catch (...) {
    B2ERROR("Exception during vertex fit.");
    return false;
  }

  const bool ok = (vertexFit.doFit() == analysis::KFitError::kNoError);
  if (!ok) return false;

  const HepGeom::Point3D<double> posVertex = vertexFit.getVertex();
  const CLHEP::HepSymMatrix covVertex = vertexFit.getVertexError();
  const TVector3 extrapolationTarget(posVertex.x(), posVertex.y(), posVertex.z());

  std::vector<genfit::GFRaveTrackParameters*> trackParamsVertex;
  trackParamsVertex.reserve(2);
  for (int i = 0; i <= 1; ++i) {
    const genfit::Track& daughter = i == 0 ? trackPlus : trackMinus;
    // KFit gives back the daughter parameters at the reference point of the track:
    // the momentum there points elsewhere than at the vertex. Transport the daughter
    // to the vertex position with genfit instead.
    TVector3 pos;
    TVector3 mom;
    TMatrixDSym cov6;
    try {
      genfit::MeasuredStateOnPlane daughterState = daughter.getFittedState();
      daughterState.extrapolateToPoint(extrapolationTarget);
      daughterState.getPosMomCov(pos, mom, cov6);
    } catch (...) {
      B2ERROR("Exception while extrapolating a daughter to the fitted vertex.");
      for (auto* trackParams : trackParamsVertex) delete trackParams;
      return false;
    }
    TVectorD state{6};
    state[0] = pos.X();
    state[1] = pos.Y();
    state[2] = pos.Z();
    state[3] = mom.X();
    state[4] = mom.Y();
    state[5] = mom.Z();
    genfit::GFRaveTrackParameters* trackParams = new genfit::GFRaveTrackParameters(nullptr, nullptr, 1, state, cov6, true);
    trackParamsVertex.push_back(trackParams);
  }
  const double ndfVertex = static_cast<double>(vertexFit.getNDF());
  const double chisqVertex = vertexFit.getCHIsq();

  vertex = genfit::GFRaveVertex{TVector3{posVertex.x(), posVertex.y(), posVertex.z()}, CLHEPToROOT::getTMatrixDSym(covVertex), trackParamsVertex, ndfVertex, chisqVertex};

  return true;
}
