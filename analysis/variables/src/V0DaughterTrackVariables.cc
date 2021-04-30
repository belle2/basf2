/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kota Nakagiri, Yuma Uematsu                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own includes
#include <analysis/variables/V0DaughterTrackVariables.h>
#include <analysis/VariableManager/Manager.h>

#include <analysis/variables/TrackVariables.h>

// framework - DataStore
#include <framework/dataobjects/Helix.h>

// dataobjects from the MDST
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>

// framework aux
#include <framework/logging/Logger.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    // helper function to get the helix parameters of the V0 daughter tracks
    // Not registered in variable manager
    double getV0DaughterTrackDetNHits(const Particle* particle, const double daughterID, const Const::EDetector& det)
    {
      auto daughter = particle->getDaughter(daughterID);
      return trackNHits(daughter, det);
    }

    double v0DaughterTrackNCDCHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackDetNHits(part, daughterID[0], Const::EDetector::CDC);
    }

    double v0DaughterTrackNSVDHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackDetNHits(part, daughterID[0], Const::EDetector::SVD);
    }

    double v0DaughterTrackNPXDHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackDetNHits(part, daughterID[0], Const::EDetector::PXD);
    }

    double v0DaughterTrackNVXDHits(const Particle* part, const std::vector<double>& daughterID)
    {
      return v0DaughterTrackNPXDHits(part, daughterID) + v0DaughterTrackNSVDHits(part, daughterID);
    }

    double v0DaughterTrackFirstSVDLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackFirstSVDLayer(daughter);
    }

    double v0DaughterTrackFirstPXDLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackFirstPXDLayer(daughter);
    }

    double v0DaughterTrackFirstCDCLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackFirstCDCLayer(daughter);
    }

    double v0DaughterTrackLastCDCLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackLastCDCLayer(daughter);
    }

    double v0DaughterTrackPValue(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackPValue(daughter);
    }

    double v0DaughterTrackD0(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackD0(daughter);
    }

    double v0DaughterTrackPhi0(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackPhi0(daughter);
    }

    double v0DaughterTrackOmega(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackOmega(daughter);
    }

    double v0DaughterTrackZ0(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackZ0(daughter);
    }

    double v0DaughterTrackTanLambda(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackTanLambda(daughter);
    }

    double v0DaughterTrackD0Error(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackD0Error(daughter);
    }

    double v0DaughterTrackPhi0Error(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackPhi0Error(daughter);
    }

    double v0DaughterTrackOmegaError(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackOmegaError(daughter);
    }

    double v0DaughterTrackZ0Error(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackZ0Error(daughter);
    }

    double v0DaughterTrackTanLambdaError(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return trackTanLambdaError(daughter);
    }

    double v0DaughterD0(const Particle* particle, const std::vector<double>& daughterID)
    {
      if (!particle)
        return std::numeric_limits<float>::quiet_NaN();

      TVector3 v0Vertex = particle->getVertex();

      const Particle* daug = particle->getDaughter(daughterID[0]);

      const TrackFitResult* trackFit = daug->getTrackFitResult();
      if (!trackFit) return std::numeric_limits<float>::quiet_NaN();

      UncertainHelix helix = trackFit->getUncertainHelix();
      helix.passiveMoveBy(v0Vertex);

      return helix.getD0();
    }

    double v0DaughterD0Diff(const Particle* particle)
    {
      return v0DaughterD0(particle, {0}) - v0DaughterD0(particle, {1});
    }

    double v0DaughterZ0(const Particle* particle, const std::vector<double>& daughterID)
    {
      if (!particle)
        return std::numeric_limits<float>::quiet_NaN();

      TVector3 v0Vertex = particle->getVertex();

      const Particle* daug = particle->getDaughter(daughterID[0]);

      const TrackFitResult* trackFit = daug->getTrackFitResult();
      if (!trackFit) return std::numeric_limits<float>::quiet_NaN();

      UncertainHelix helix = trackFit->getUncertainHelix();
      helix.passiveMoveBy(v0Vertex);

      return helix.getZ0();
    }

    double v0DaughterZ0Diff(const Particle* particle)
    {
      return v0DaughterZ0(particle, {0}) - v0DaughterZ0(particle, {1});
    }

    // helper function to get pull of the helix parameters of the V0 daughter tracks with the true vertex as the pivot.
    // Not registered in variable manager
    double getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(const Particle* particle, const double daughterID,
        const int tauIndex)
    {
      if (!particle) { return std::numeric_limits<double>::quiet_NaN(); }

      const int dID = int(std::lround(daughterID));
      if (not(dID == 0 || dID == 1)) { return std::numeric_limits<double>::quiet_NaN(); }

      const MCParticle* mcparticle_v0 = particle->getMCParticle();
      if (!mcparticle_v0) { return std::numeric_limits<double>::quiet_NaN(); }

      if (!(particle->getDaughter(dID))) { return std::numeric_limits<double>::quiet_NaN(); }

      const MCParticle* mcparticle = particle->getDaughter(dID)->getMCParticle();
      if (!mcparticle) { return std::numeric_limits<double>::quiet_NaN(); }

      const TrackFitResult* trackFit = particle->getDaughter(dID)->getTrackFitResult();
      if (!trackFit) { return std::numeric_limits<double>::quiet_NaN(); }

      // MC information
      const TVector3 mcProdVertex   = mcparticle->getVertex();
      const TVector3 mcMomentum     = mcparticle->getMomentum();
      const double mcParticleCharge = mcparticle->getCharge();
      const double BzAtProdVertex = BFieldManager::getFieldInTesla(mcProdVertex).Z();
      Helix mcHelix = Helix(mcProdVertex, mcMomentum, mcParticleCharge, BzAtProdVertex);
      mcHelix.passiveMoveBy(mcProdVertex);
      const std::vector<double> mcHelixPars = { mcHelix.getD0(), mcHelix.getPhi0(), mcHelix.getOmega(),
                                                mcHelix.getZ0(), mcHelix.getTanLambda()
                                              };

      // measured information (from the reconstructed particle)
      UncertainHelix measHelix = trackFit->getUncertainHelix();
      measHelix.passiveMoveBy(mcProdVertex);
      const TMatrixDSym measCovariance = measHelix.getCovariance();
      const std::vector<double> measHelixPars = {measHelix.getD0(), measHelix.getPhi0(), measHelix.getOmega(),
                                                 measHelix.getZ0(), measHelix.getTanLambda()
                                                };
      const std::vector<double> measErrSquare = {measCovariance[0][0], measCovariance[1][1], measCovariance[2][2],
                                                 measCovariance[3][3], measCovariance[4][4]
                                                };

      if (measErrSquare.at(tauIndex) > 0)
        return (mcHelixPars.at(tauIndex) - measHelixPars.at(tauIndex)) / std::sqrt(measErrSquare.at(tauIndex));
      else
        return std::numeric_limits<double>::quiet_NaN();
    }

    double v0DaughterHelixWithTrueVertexAsPivotD0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(part, daughterID[0], 0);
    }

    double v0DaughterHelixWithTrueVertexAsPivotPhi0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(part, daughterID[0], 1);
    }

    double v0DaughterHelixWithTrueVertexAsPivotOmegaPull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(part, daughterID[0], 2);
    }

    double v0DaughterHelixWithTrueVertexAsPivotZ0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(part, daughterID[0], 3);
    }

    double v0DaughterHelixWithTrueVertexAsPivotTanLambdaPull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(part, daughterID[0], 4);
    }

    double v0DaughterHelixWithOriginAsPivotD0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return getHelixD0Pull(daughter);
    }

    double v0DaughterHelixWithOriginAsPivotPhi0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return getHelixPhi0Pull(daughter);
    }

    double v0DaughterHelixWithOriginAsPivotOmegaPull(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return getHelixOmegaPull(daughter);
    }

    double v0DaughterHelixWithOriginAsPivotZ0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return getHelixZ0Pull(daughter);
    }

    double v0DaughterHelixWithOriginAsPivotTanLambdaPull(const Particle* part, const std::vector<double>& daughterID)
    {
      auto daughter = part->getDaughter(daughterID[0]);
      return getHelixTanLambdaPull(daughter);
    }

    double v0DaughterTrackParam5AtIPPerigee(const Particle* part, const std::vector<double>& params)
    {
      auto daughter = part->getDaughter(params[0]);
      if (!daughter) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      auto trackFit = daughter->getTrackFitResult();
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int paramID = int(std::lround(params[1]));
      if (not(0 <= paramID && paramID < 5))
        return std::numeric_limits<double>::quiet_NaN();

      std::vector<float> tau = trackFit->getTau();
      return tau[paramID];
    }

    double v0DaughterTrackParamCov5x5AtIPPerigee(const Particle* part, const std::vector<double>& params)
    {
      auto daughter = part->getDaughter(params[0]);
      if (!daughter) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      auto trackFit = daughter->getTrackFitResult();
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      const int paramID = int(std::lround(params[1]));
      if (not(0 <= paramID && paramID < 15))
        return std::numeric_limits<double>::quiet_NaN();

      std::vector<float> cov = trackFit->getCov();
      return cov[paramID];
    }


    VARIABLE_GROUP("V0Daughter");

    REGISTER_VARIABLE("v0DaughterNCDCHits(i)", v0DaughterTrackNCDCHits, "Number of CDC hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNCDCHits(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nCDCHits`,
                     so replace the current call with ``daughter(i, nCDCHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterNSVDHits(i)", v0DaughterTrackNSVDHits, "Number of SVD hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNSVDHits(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nSVDHits`,
                     so replace the current call with ``daughter(i, nSVDHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterNPXDHits(i)", v0DaughterTrackNPXDHits, "Number of PXD hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNPXDHits(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nPXDHits`,
                     so replace the current call with ``daughter(i, nPXDHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterNVXDHits(i)", v0DaughterTrackNVXDHits, "Number of PXD+SVD hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNVXDHits(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nVXDHits`,
                     so replace the current call with ``daughter(i, nVXDHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterFirstSVDLayer(i)", v0DaughterTrackFirstSVDLayer,
                      "First activated SVD layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterFirstSVDLayer(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `firstSVDLayer`,
                     so replace the current call with ``daughter(i, firstSVDLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterFirstPXDLayer(i)", v0DaughterTrackFirstPXDLayer,
                      "First activated PXD layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterFirstPXDLayer(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `firstPXDLayer`,
                     so replace the current call with ``daughter(i, firstPXDLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterFirstCDCLayer(i)", v0DaughterTrackFirstCDCLayer,
                      "First activated CDC layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterFirstCDCLayer(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `firstCDCLayer`,
                     so replace the current call with ``daughter(i, firstCDCLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterLastCDCLayer(i)",  v0DaughterTrackLastCDCLayer,
                      "Last CDC layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterLastCDCLayer(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `lastCDCLayer`,
                     so replace the current call with ``daughter(i, lastCDCLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPValue(i)",        v0DaughterTrackPValue,
                      "chi2 probalility of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterPValue(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `pValue`,
                     so replace the current call with ``daughter(i, pValue)``.)DOC");
    /// helix parameters
    REGISTER_VARIABLE("v0DaughterD0(i)",        v0DaughterTrackD0,        "d0 of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterD0(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `d0`,
                     so replace the current call with ``daughter(i, d0)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPhi0(i)",      v0DaughterTrackPhi0,      "phi0 of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterPhi0(i)", false, "light-2104-poseidon", R"DOC(
                 The same value can be calculated with the more generic variable `phi0`,
                 so replace the current call with ``daughter(i, phi0)``.)DOC");
    REGISTER_VARIABLE("v0DaughterOmega(i)",     v0DaughterTrackOmega,     "omega of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterOmega(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `omega`,
                     so replace the current call with ``daughter(i, omega)``.)DOC");
    REGISTER_VARIABLE("v0DaughterZ0(i)",        v0DaughterTrackZ0,        "z0 of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterZ0(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `z0`,
                     so replace the current call with ``daughter(i, z0)``.)DOC");
    REGISTER_VARIABLE("v0DaughterTanLambda(i)", v0DaughterTrackTanLambda, "tan(lambda) of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterTanLambda(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `tanLambda`,
                     so replace the current call with ``daughter(i, tanLambda)``.)DOC");
    /// error of helix parameters
    REGISTER_VARIABLE("v0DaughterD0Error(i)",        v0DaughterTrackD0Error,        "d0 error of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterD0Error(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `d0Err`,
                     so replace the current call with ``daughter(i, d0Err)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPhi0Error(i)",      v0DaughterTrackPhi0Error,      "phi0 error of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterPhi0Error(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `phi0Err`,
                     so replace the current call with ``daughter(i, phi0Err)``.)DOC");
    REGISTER_VARIABLE("v0DaughterOmegaError(i)",     v0DaughterTrackOmegaError,     "omega error of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterOmegaError(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `omegaErr`,
                     so replace the current call with ``daughter(i, omegaErr)``.)DOC");
    REGISTER_VARIABLE("v0DaughterZ0Error(i)",        v0DaughterTrackZ0Error,        "z0 error of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterZ0Error(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `z0Err`,
                     so replace the current call with ``daughter(i, z0Err)``.)DOC");
    REGISTER_VARIABLE("v0DaughterTanLambdaError(i)", v0DaughterTrackTanLambdaError, "tan(lambda) error of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterTanLambdaError(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `tanLambdaErr`,
                     so replace the current call with ``daughter(i, tanLambdaErr)``.)DOC");

    /// V0 daughter helix parameters with V0 vertex as pivot
    REGISTER_VARIABLE("V0d0(id)", v0DaughterD0,
                      "Return the d0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track.");
    REGISTER_VARIABLE("V0Deltad0", v0DaughterD0Diff,
                      "Return the difference between d0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track.");
    REGISTER_VARIABLE("V0z0(id)", v0DaughterZ0,
                      "Return the z0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track.");
    REGISTER_VARIABLE("V0Deltaz0", v0DaughterZ0Diff,
                      "Return the difference between z0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track.");

    /// pull of helix parameters with the reco. vertex as the pivot
    REGISTER_VARIABLE("v0DaughterD0PullWithTrueVertexAsPivot(i)",       v0DaughterHelixWithTrueVertexAsPivotD0Pull,
                      "d0 pull of the i-th daughter track with the true V0 vertex as the track pivot");
    REGISTER_VARIABLE("v0DaughterPhi0PullWithTrueVertexAsPivot(i)",     v0DaughterHelixWithTrueVertexAsPivotPhi0Pull,
                      "phi0 pull of the i-th daughter track with the true V0 vertex as the track pivot");
    REGISTER_VARIABLE("v0DaughterOmegaPullWithTrueVertexAsPivot(i)",    v0DaughterHelixWithTrueVertexAsPivotOmegaPull,
                      "omega pull of the i-th daughter track with the true V0 vertex as the track pivot");
    REGISTER_VARIABLE("v0DaughterZ0PullWithTrueVertexAsPivot(i)",       v0DaughterHelixWithTrueVertexAsPivotZ0Pull,
                      "z0 pull of the i-th daughter track with the true V0 vertex as the track pivot");
    REGISTER_VARIABLE("v0DaughterTanLambdaPullWithTrueVertexAsPivot(i)", v0DaughterHelixWithTrueVertexAsPivotTanLambdaPull,
                      "tan(lambda) pull of the i-th daughter track with the true V0 vertex as the track pivot");
    /// pull of helix parameters with the origin as the pivot
    REGISTER_VARIABLE("v0DaughterD0PullWithOriginAsPivot(i)",       v0DaughterHelixWithOriginAsPivotD0Pull,
                      "d0 pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterD0PullWithOriginAsPivot(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `d0Pull`,
                     so replace the current call with ``daughter(i, d0Pull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPhi0PullWithOriginAsPivot(i)",     v0DaughterHelixWithOriginAsPivotPhi0Pull,
                      "phi0 pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterPhi0PullWithOriginAsPivot(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `phi0Pull`,
                     so replace the current call with ``daughter(i, phi0Pull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterOmegaPullWithOriginAsPivot(i)",    v0DaughterHelixWithOriginAsPivotOmegaPull,
                      "omega pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterOmegaPullWithOriginAsPivot(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `omegaPull`,
                     so replace the current call with ``daughter(i, omegaPull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterZ0PullWithOriginAsPivot(i)",       v0DaughterHelixWithOriginAsPivotZ0Pull,
                      "z0 pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterZ0PullWithOriginAsPivot(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `z0Pull`,
                     so replace the current call with ``daughter(i, z0Pull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterTanLambdaPullWithOriginAsPivot(i)", v0DaughterHelixWithOriginAsPivotTanLambdaPull,
                      "tan(lambda) pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterTanLambdaPullWithOriginAsPivot(i)", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `tanLambdaPull`,
                     so replace the current call with ``daughter(i, tanLambdaPull)``.)DOC");
    /// helix parameters and covariance matrix elements
    REGISTER_VARIABLE("v0DaughterTau(i,j)",        v0DaughterTrackParam5AtIPPerigee,
                      "j-th track parameter (at IP perigee) of the i-th daughter track. "
                      "j:  0:d0, 1:phi0, 2:omega, 3:z0, 4:tanLambda");
    REGISTER_VARIABLE("v0DaughterCov(i,j)",        v0DaughterTrackParamCov5x5AtIPPerigee,
                      "j-th element of the 15 covariance matrix elements (at IP perigee) of the i-th daughter track. "
                      "(0,0), (0,1) ... (1,1), (1,2) ... (2,2) ...");
  }
}
