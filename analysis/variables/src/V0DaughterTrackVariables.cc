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

#include <analysis/variables/TrackVariables.h>// getTrackFitResultFromParticle

// framework - DataStore
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Helix.h>

// dataobjects from the MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/V0.h>
#include <mdst/dataobjects/EventLevelTrackingInfo.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>
#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/logging/Logger.h>

#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace Variable {

    // An helper function to get track fit results
    // Not registered in variable mananger
    const TrackFitResult* getTrackFitResultFromV0DaughterParticle(const Particle* particle, const double daughterID)
    {
      const int dID = int(std::lround(daughterID));
      if (not(dID == 0 || dID == 1))
        return nullptr;

      const Particle* daughter = particle->getDaughter(dID);
      if (!daughter)
        return nullptr;

      const TrackFitResult* trackFit = daughter->getRelated<TrackFitResult>();

      return trackFit;
    }

    // helper function to get the helix parameters of the V0 daughter tracks
    // Not registered in variable mananger
    double getV0DaughterTrackDetNHits(const Particle* particle, const double daughterID, const Const::EDetector& det)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(particle, daughterID);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }

      if (det == Const::EDetector::CDC) {
        return trackFit->getHitPatternCDC().getNHits();
      } else if (det == Const::EDetector::SVD) {
        return trackFit->getHitPatternVXD().getNSVDHits();
      } else if (det == Const::EDetector::PXD) {
        return trackFit->getHitPatternVXD().getNPXDHits();
      } else {
        return std::numeric_limits<double>::quiet_NaN();
      }
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
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternVXD().getFirstSVDLayer();
    }

    double v0DaughterTrackFirstPXDLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternVXD().getFirstPXDLayer(HitPatternVXD::PXDMode::normal);
    }

    double v0DaughterTrackFirstCDCLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternCDC().getFirstLayer();
    }

    double v0DaughterTrackLastCDCLayer(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getHitPatternCDC().getLastLayer();
    }

    double v0DaughterTrackPValue(const Particle* part, const std::vector<double>& daughterID)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, daughterID[0]);
      if (!trackFit) {
        return std::numeric_limits<float>::quiet_NaN();
      }
      return trackFit->getPValue();
    }

    // helper function to get the helix parameters of the V0 daughter tracks
    // Not registered in variable mananger
    double getV0DaughterTrackParamAtIndex(const Particle* particle, const double daughterID, const int tauIndex)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(particle, daughterID);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      return trackFit->getTau().at(tauIndex);
    }

    double v0DaughterTrackD0(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamAtIndex(part, daughterID[0], 0);
    }

    double v0DaughterTrackPhi0(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamAtIndex(part, daughterID[0], 1);
    }

    double v0DaughterTrackOmega(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamAtIndex(part, daughterID[0], 2);
    }

    double v0DaughterTrackZ0(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamAtIndex(part, daughterID[0], 3);
    }

    double v0DaughterTrackTanLambda(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamAtIndex(part, daughterID[0], 4);
    }

    // helper function to get the error of the helix parameters of the V0 daughter tracks
    // Not registered in variable mananger
    double getV0DaughterTrackParamErrorAtIndex(const Particle* particle, const double daughterID, const int tauIndex)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(particle, daughterID);
      if (!trackFit) {
        return std::numeric_limits<double>::quiet_NaN();
      }
      double errorSquared = trackFit->getCovariance5()[tauIndex][tauIndex];
      if (errorSquared > 0)
        return sqrt(errorSquared);
      else
        return std::numeric_limits<double>::quiet_NaN();
    }

    double v0DaughterTrackD0Error(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamErrorAtIndex(part, daughterID[0], 0);
    }

    double v0DaughterTrackPhi0Error(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamErrorAtIndex(part, daughterID[0], 1);
    }

    double v0DaughterTrackOmegaError(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamErrorAtIndex(part, daughterID[0], 2);
    }

    double v0DaughterTrackZ0Error(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamErrorAtIndex(part, daughterID[0], 3);
    }

    double v0DaughterTrackTanLambdaError(const Particle* part, const std::vector<double>& daughterID)
    {
      return getV0DaughterTrackParamErrorAtIndex(part, daughterID[0], 4);
    }

    // helper function to get pull of the helix parameters of the V0 daughter tracks with the true vertex as the pivot.
    // Not registered in variable mananger
    double getHelixParameterPullOfV0DaughterWithTrueVertexAsPivotAtIndex(const Particle* particle, const double daughterID,
        const int tauIndex)
    {
      if (!particle) { return std::numeric_limits<double>::quiet_NaN(); }

      const int dID = int(std::lround(daughterID));
      if (not(dID == 0 || dID == 1)) { return std::numeric_limits<double>::quiet_NaN(); }

      const MCParticle* mcparticle_v0 = particle->getRelatedTo<MCParticle>();
      if (!mcparticle_v0) { return std::numeric_limits<double>::quiet_NaN(); }

      if (!(particle->getDaughter(dID))) { return std::numeric_limits<double>::quiet_NaN(); }

      const MCParticle* mcparticle = particle->getDaughter(dID)->getRelatedTo<MCParticle>();
      if (!mcparticle) { return std::numeric_limits<double>::quiet_NaN(); }

      const TrackFitResult* trackFit = getTrackFitResultFromV0DaughterParticle(particle, daughterID);
      if (!trackFit) { return std::numeric_limits<double>::quiet_NaN(); }

      // MC information
      const TVector3 mcProdVertex   = mcparticle->getVertex();
      const TVector3 mcMomentum     = mcparticle->getMomentum();
      const double mcParticleCharge = mcparticle->getCharge();
      const double BzAtProdVertex = BFieldManager::getField(TVector3(mcProdVertex.X(), mcProdVertex.Y(),
                                                            mcProdVertex.Z())).Z() / Belle2::Unit::T;
      Helix mcHelix = Helix(mcProdVertex, mcMomentum, mcParticleCharge, BzAtProdVertex);
      mcHelix.passiveMoveBy(mcProdVertex);
      const std::vector<double> mcHelixPars = { mcHelix.getD0(), mcHelix.getPhi0(), mcHelix.getOmega(),
                                                mcHelix.getZ0(), mcHelix.getTanLambda()
                                              };

      // measured information (from the rconstructed particle)
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

    // helper function to get pull of the helix parameters of the V0 daughter tracks with the origin as the pivot.
    // Not registered in variable mananger
    double getHelixParameterPullOfV0DaughterWithOriginAsPivotAtIndex(const Particle* particle, const double daughterID,
        const int tauIndex)
    {
      if (!particle) { return std::numeric_limits<double>::quiet_NaN(); }

      const int dID = int(std::lround(daughterID));
      if (not(dID == 0 || dID == 1)) { return std::numeric_limits<double>::quiet_NaN(); }

      const MCParticle* mcparticle_v0 = particle->getRelatedTo<MCParticle>();
      if (!mcparticle_v0) { return std::numeric_limits<double>::quiet_NaN(); }

      if (!(particle->getDaughter(dID))) { return std::numeric_limits<double>::quiet_NaN(); }

      const MCParticle* mcparticle = particle->getDaughter(dID)->getRelatedTo<MCParticle>();
      if (!mcparticle) { return std::numeric_limits<double>::quiet_NaN(); }

      const TrackFitResult* trackFit = getTrackFitResultFromV0DaughterParticle(particle, daughterID);
      if (!trackFit) { return std::numeric_limits<double>::quiet_NaN(); }

      // MC information
      const TVector3 mcProdVertex   = mcparticle->getVertex();
      const TVector3 mcMomentum     = mcparticle->getMomentum();
      const double mcParticleCharge = mcparticle->getCharge();
      const double BzAtOrigin = BFieldManager::getField(TVector3(0, 0, 0)).Z() / Belle2::Unit::T;

      Helix mcHelix = Helix(mcProdVertex, mcMomentum, mcParticleCharge, BzAtOrigin);
      const std::vector<double> mcHelixPars = { mcHelix.getD0(), mcHelix.getPhi0(), mcHelix.getOmega(),
                                                mcHelix.getZ0(), mcHelix.getTanLambda()
                                              };

      // measured information (from the rconstructed particle)
      UncertainHelix measHelix = trackFit->getUncertainHelix();
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

    double v0DaughterHelixWithOriginAsPivotD0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithOriginAsPivotAtIndex(part, daughterID[0], 0);
    }

    double v0DaughterHelixWithOriginAsPivotPhi0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithOriginAsPivotAtIndex(part, daughterID[0], 1);
    }

    double v0DaughterHelixWithOriginAsPivotOmegaPull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithOriginAsPivotAtIndex(part, daughterID[0], 2);
    }

    double v0DaughterHelixWithOriginAsPivotZ0Pull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithOriginAsPivotAtIndex(part, daughterID[0], 3);
    }

    double v0DaughterHelixWithOriginAsPivotTanLambdaPull(const Particle* part, const std::vector<double>& daughterID)
    {
      return getHelixParameterPullOfV0DaughterWithOriginAsPivotAtIndex(part, daughterID[0], 4);
    }

    double v0DaughterTrackParam5AtIPPerigee(const Particle* part, const std::vector<double>& params)
    {
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, params[0]);
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
      auto trackFit = getTrackFitResultFromV0DaughterParticle(part, params[0]);
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
    REGISTER_VARIABLE("v0DaughterNSVDHits(i)", v0DaughterTrackNSVDHits, "Number of SVD hits associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterNPXDHits(i)", v0DaughterTrackNPXDHits, "Number of PXD hits associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterNVXDHits(i)", v0DaughterTrackNVXDHits, "Number of PXD+SVD hits associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterFirstSVDLayer(i)", v0DaughterTrackFirstSVDLayer,
                      "First activated SVD layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterFirstPXDLayer(i)", v0DaughterTrackFirstPXDLayer,
                      "First activated PXD layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterFirstCDCLayer(i)", v0DaughterTrackFirstCDCLayer,
                      "First activated CDC layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterLastCDCLayer(i)",  v0DaughterTrackLastCDCLayer,
                      "Last CDC layer associated to the i-th daughter track");
    REGISTER_VARIABLE("v0DaughterPValue(i)",        v0DaughterTrackPValue,
                      "chi2 probalility of the i-th daughter track fit");
    /// helix parameters
    REGISTER_VARIABLE("v0DaughterD0(i)",        v0DaughterTrackD0,        "d0 of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterPhi0(i)",      v0DaughterTrackPhi0,      "phi0 of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterOmega(i)",     v0DaughterTrackOmega,     "omega of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterZ0(i)",        v0DaughterTrackZ0,        "z0 of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterTanLambda(i)", v0DaughterTrackTanLambda, "tan(lambda) of the i-th daughter track fit");
    /// error of helix parameters
    REGISTER_VARIABLE("v0DaughterD0Error(i)",        v0DaughterTrackD0Error,        "d0 error of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterPhi0Error(i)",      v0DaughterTrackPhi0Error,      "phi0 error of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterOmegaError(i)",     v0DaughterTrackOmegaError,     "omega error of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterZ0Error(i)",        v0DaughterTrackZ0Error,        "z0 error of the i-th daughter track fit");
    REGISTER_VARIABLE("v0DaughterTanLambdaError(i)", v0DaughterTrackTanLambdaError, "tan(lambda) error of the i-th daughter track fit");
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
    REGISTER_VARIABLE("v0DaughterPhi0PullWithOriginAsPivot(i)",     v0DaughterHelixWithOriginAsPivotPhi0Pull,
                      "phi0 pull of the i-th daughter track with the origin as the track pivot");
    REGISTER_VARIABLE("v0DaughterOmegaPullWithOriginAsPivot(i)",    v0DaughterHelixWithOriginAsPivotOmegaPull,
                      "omega pull of the i-th daughter track with the origin as the track pivot");
    REGISTER_VARIABLE("v0DaughterZ0PullWithOriginAsPivot(i)",       v0DaughterHelixWithOriginAsPivotZ0Pull,
                      "z0 pull of the i-th daughter track with the origin as the track pivot");
    REGISTER_VARIABLE("v0DaughterTanLambdaPullWithOriginAsPivot(i)", v0DaughterHelixWithOriginAsPivotTanLambdaPull,
                      "tan(lambda) pull of the i-th daughter track with the origin as the track pivot");
    /// helix parameters and covariance matrix elements
    REGISTER_VARIABLE("v0DaughterTau(i,j)",        v0DaughterTrackParam5AtIPPerigee,
                      "j-th track parameter (at IP perigee) of the i-th daughter track. "
                      "j:  0:d0, 1:phi0, 2:omega, 3:z0, 4:tanLambda");
    REGISTER_VARIABLE("v0DaughterCov(i,j)",        v0DaughterTrackParamCov5x5AtIPPerigee,
                      "j-th element of the 15 covariance matrix elements (at IP perigee) of the i-th daughter track. "
                      "(0,0), (0,1) ... (1,1), (1,2) ... (2,2) ...");
  }
}
