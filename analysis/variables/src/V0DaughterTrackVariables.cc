/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own header.
#include <analysis/variables/V0DaughterTrackVariables.h>

// include VariableManager
#include <analysis/VariableManager/Manager.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/variables/TrackVariables.h>

// framework - DataStore
#include <framework/dataobjects/Helix.h>

// dataobjects from the MDST
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/HitPatternVXD.h>

// framework aux
#include <framework/logging/Logger.h>

#include <algorithm>
#include <TVector2.h>
#include <cmath>

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

    double v0DaughterTrackNRemovedHits(const Particle* part, const std::vector<double>& daughterID)
    {
      // returns 0 if called for non-V0 particles
      if (part->getParticleSource() != Particle::EParticleSourceObject::c_V0)
        return 0;
      auto daughter = part->getDaughter(daughterID[0]);
      const Track* track = daughter->getTrack();
      if (!track) return Const::doubleNaN;
      const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(Const::ChargedStable(abs(
                                         daughter->getPDGCode())));
      if (!trackFit) return Const::doubleNaN;
      double nHitsBeforeRemoval = trackFit->getHitPatternCDC().getNHits()
                                  + trackFit->getHitPatternVXD().getNSVDHits()
                                  + trackFit->getHitPatternVXD().getNPXDHits();
      double nHitsAfterRemoval = trackNVXDHits(daughter) + trackNCDCHits(daughter);
      return nHitsBeforeRemoval - nHitsAfterRemoval;
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
        return Const::doubleNaN;

      ROOT::Math::XYZVector v0Vertex = particle->getVertex();

      const Particle* daug = particle->getDaughter(daughterID[0]);

      const TrackFitResult* trackFit = daug->getTrackFitResult();
      if (!trackFit) return Const::doubleNaN;

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
        return Const::doubleNaN;

      ROOT::Math::XYZVector v0Vertex = particle->getVertex();

      const Particle* daug = particle->getDaughter(daughterID[0]);

      const TrackFitResult* trackFit = daug->getTrackFitResult();
      if (!trackFit) return Const::doubleNaN;

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
      if (!particle) { return Const::doubleNaN; }

      const int dID = int(std::lround(daughterID));
      if (not(dID == 0 || dID == 1)) { return Const::doubleNaN; }

      const MCParticle* mcparticle_v0 = particle->getMCParticle();
      if (!mcparticle_v0) { return Const::doubleNaN; }

      if (!(particle->getDaughter(dID))) { return Const::doubleNaN; }

      const MCParticle* mcparticle = particle->getDaughter(dID)->getMCParticle();
      if (!mcparticle) { return Const::doubleNaN; }

      const TrackFitResult* trackFit = particle->getDaughter(dID)->getTrackFitResult();
      if (!trackFit) { return Const::doubleNaN; }

      // MC information
      const ROOT::Math::XYZVector mcProdVertex   = mcparticle->getVertex();
      const ROOT::Math::XYZVector mcMomentum     = mcparticle->getMomentum();
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
        return Const::doubleNaN;
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
        return Const::doubleNaN;
      }
      auto trackFit = daughter->getTrackFitResult();
      if (!trackFit) {
        return Const::doubleNaN;
      }

      const int paramID = int(std::lround(params[1]));
      if (not(0 <= paramID && paramID < 5))
        return Const::doubleNaN;

      std::vector<float> tau = trackFit->getTau();
      return tau[paramID];
    }

    double v0DaughterTrackParamCov5x5AtIPPerigee(const Particle* part, const std::vector<double>& params)
    {
      auto daughter = part->getDaughter(params[0]);
      if (!daughter) {
        return Const::doubleNaN;
      }
      auto trackFit = daughter->getTrackFitResult();
      if (!trackFit) {
        return Const::doubleNaN;
      }

      const int paramID = int(std::lround(params[1]));
      if (not(0 <= paramID && paramID < 15))
        return Const::doubleNaN;

      std::vector<float> cov = trackFit->getCov();
      return cov[paramID];
    }

    int convertedPhotonErrorChecks(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      //Check that exactly two daughter indices are provided
      if (daughterIndices.size() != 2) {
        B2ERROR("Invalid number of daughter indices. Please specify exactly two valid daughter indices.");
        return -1;
      }

      //Check that there are at least (r+1) daughters where r is the bigger of the two indices provided
      int daughterIndex1 = int(daughterIndices[0]);
      int daughterIndex2 = int(daughterIndices[1]);
      if (int(gamma->getNDaughters()) <= std::max(daughterIndex1, daughterIndex2)) {
        B2ERROR("Invalid daughter indices provided. Particle does not have that many daughters.");
        return -1;
      }

      //Check that there exists tracks associated with the daughter indices provided
      if (!gamma->getDaughter(daughterIndex1)->getTrack()) {
        B2ERROR("There is no track associated with daughter index " << daughterIndex1);
        return -1;
      }
      if (!gamma->getDaughter(daughterIndex2)->getTrack()) {
        B2ERROR("There is no track associated with daughter index " << daughterIndex2);
        return -1;
      }

      //Check whether tracks used to calculate variable has been reconstructed as electrons/positrons or not (INCONSEQUENTIAL)
      if (fabs(gamma->getDaughter(daughterIndex1)->getPDGCode()) != 11) {
        B2INFO("The first track provided has not been reconstructed as an electron/positron. It has PDG code " << gamma->getDaughter(
                 daughterIndex1)->getPDGCode() << ". However, this is still fully admissible.");
      }
      if (fabs(gamma->getDaughter(daughterIndex2)->getPDGCode()) != 11) {
        B2INFO("The second track provided has not been reconstructed as an electron/positron. It has PDG code " << gamma->getDaughter(
                 daughterIndex1)->getPDGCode() << ".However, this is still fully admissible.");
      }

      return 0;
    }


    int convertedPhotonLoadHelixParams(const Particle* gamma, int daughterIndex1, int daughterIndex2, double& Phi01, double& D01,
                                       double& Omega1, double& Z01, double& TanLambda1, double& Phi02, double& D02, double& Omega2, double& Z02,
                                       double& TanLambda2)
    {
      //Get helix parameters
      //Electron/track 1
      Helix e1Helix = gamma->getDaughter(daughterIndex1)->getTrackFitResult()->getHelix();

      Phi01 = e1Helix.getPhi0();
      D01  = e1Helix.getD0() ;
      Omega1  = e1Helix.getOmega();
      Z01   = e1Helix.getZ0();
      TanLambda1 = e1Helix.getTanLambda();

      //Electron/track 2
      Helix e2Helix = gamma->getDaughter(daughterIndex2)->getTrackFitResult()->getHelix();

      Phi02 = e2Helix.getPhi0();
      D02  = e2Helix.getD0() ;
      Omega2  = e2Helix.getOmega();
      Z02   = e2Helix.getZ0();
      TanLambda2 = e2Helix.getTanLambda();

      //Check if either track has zero curvature
      if (Omega1 == 0) {return -1;}
      else if (Omega2 == 0) {return -2;}
      else {return 0;}

    }

    double convertedPhotonInvariantMass(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      //Do basic checks
      int errFlag = convertedPhotonErrorChecks(gamma, daughterIndices);
      if (errFlag == -1) {return Const::doubleNaN;}

      //Load helix parameters
      double Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02, Omega2, Z02, TanLambda2;
      int daughterIndex1 = int(daughterIndices[0]);
      int daughterIndex2 = int(daughterIndices[1]);
      convertedPhotonLoadHelixParams(gamma, daughterIndex1, daughterIndex2, Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02,
                                     Omega2, Z02, TanLambda2);


      //Calculating invariant mass
      //Sine and cosine Lambda
      double sinlam1 = TanLambda1 / sqrt(1 + (TanLambda1 * TanLambda1));
      double coslam1 = 1 / sqrt(1 + (TanLambda1 * TanLambda1));
      double sinlam2 = TanLambda2 / sqrt(1 + (TanLambda2 * TanLambda2));
      double coslam2 = 1 / sqrt(1 + (TanLambda2 * TanLambda2));

      //Transverse and longitudinal momentum components; energy with electron mass hypothesis
      //electron 1
      double p1  = gamma->getDaughter(daughterIndex1)->getMomentumMagnitude();
      double pt1 = p1 * coslam1, pz1 =  p1 * sinlam1;
      double e1 = sqrt((p1 * p1) + (Const::electronMass * Const::electronMass));
      //electron 2
      double p2  = gamma->getDaughter(daughterIndex2)->getMomentumMagnitude();
      double pt2 = p2 * coslam2, pz2 =  p2 * sinlam2;
      double e2 = sqrt((p2 * p2) + (Const::electronMass * Const::electronMass));

      //Invariant mass of the two track system
      double vtxMass = sqrt(pow(e1 + e2, 2.0) - pow(pt1 + pt2, 2.0)  - pow(pz1 + pz2, 2.0));
      return vtxMass;
    }

    double convertedPhotonDelTanLambda(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      //Do basic checks
      int errFlag = convertedPhotonErrorChecks(gamma, daughterIndices);
      if (errFlag == -1) {return Const::doubleNaN;}

      //Load helix parameters
      double Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02, Omega2, Z02, TanLambda2;
      int daughterIndex1 = int(daughterIndices[0]);
      int daughterIndex2 = int(daughterIndices[1]);
      convertedPhotonLoadHelixParams(gamma, daughterIndex1, daughterIndex2, Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02,
                                     Omega2, Z02, TanLambda2);



      //Delta-TanLambda
      return (TanLambda2 - TanLambda1);
    }

    double convertedPhotonDelR(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      //Do basic checks
      int errFlag = convertedPhotonErrorChecks(gamma, daughterIndices);
      if (errFlag == -1) {return Const::doubleNaN;}

      //Load helix parameters
      double Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02, Omega2, Z02, TanLambda2;
      int daughterIndex1 = int(daughterIndices[0]);
      int daughterIndex2 = int(daughterIndices[1]);
      errFlag = convertedPhotonLoadHelixParams(gamma, daughterIndex1, daughterIndex2, Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02,
                                               Omega2, Z02, TanLambda2);
      if (errFlag == -1) {
        B2ERROR("First track provided has curvature zero. Calculation of convertedPhotonDelR failed.");
        return Const::doubleNaN;
      }
      if (errFlag == -2) {
        B2ERROR("Second track provided has curvature zero. Calculation of convertedPhotonDelR failed.");
        return Const::doubleNaN;
      }

      //Delta-R
      double radius1 = 1 / Omega1;
      double radius2 = 1 / Omega2;

      TVector2 center1((radius1 + D01) * sin(Phi01), -1 * (radius1 + D01) * cos(Phi01));
      TVector2 center2((radius2 + D02) * sin(Phi02), -1 * (radius2 + D02) * cos(Phi02));
      TVector2 cenDiff = center1 - center2;

      double delR = fabs(radius1) + fabs(radius2) - cenDiff.Mod();
      return delR;
    }

    std::pair<double, double> convertedPhotonZ1Z2(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      //Do basic checks
      int errFlag = convertedPhotonErrorChecks(gamma, daughterIndices);
      if (errFlag == -1) {return std::pair<double, double>(Const::doubleNaN, Const::doubleNaN);}

      //Load helix parameters
      double Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02, Omega2, Z02, TanLambda2;
      int daughterIndex1 = int(daughterIndices[0]);
      int daughterIndex2 = int(daughterIndices[1]);
      errFlag = convertedPhotonLoadHelixParams(gamma, daughterIndex1, daughterIndex2, Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02,
                                               Omega2, Z02, TanLambda2);
      if (errFlag == -1) {
        B2ERROR("First track provided has curvature zero. Calculation of convertedPhotonZ1Z2 failed.");
        return  std::pair<double, double>(Const::doubleNaN, Const::doubleNaN);
      }
      if (errFlag == -2) {
        B2ERROR("Second track provided has curvature zero. Calculation of convertedPhotonZ1Z2 failed.");
        return  std::pair<double, double>(Const::doubleNaN, Const::doubleNaN);
      }

      //Delta-Z
      //Radial unit vectors
      double radius1 = 1 / Omega1;
      double radius2 = 1 / Omega2;

      TVector2 center1((radius1 + D01) * sin(Phi01), -1 * (radius1 + D01) * cos(Phi01));
      TVector2 center2((radius2 + D02) * sin(Phi02), -1 * (radius2 + D02) * cos(Phi02));

      TVector2 n1 =  center1 - center2; n1 = n1.Unit();
      TVector2 n2 = -1 * n1;
      n1 = copysign(1.0, Omega1) * n1;
      n2 = copysign(1.0, Omega2) * n2;

      //Getting running parameter phi at nominal vertex
      double phiN1 = atan2(n1.X(), -n1.Y());
      double phiN2 = atan2(n2.X(), -n2.Y());
      double Phi01Intersect = phiN1 - Phi01;
      double Phi02Intersect = phiN2 - Phi02;

      double z1 = Z01 - (radius1 * TanLambda1 * Phi01Intersect);
      double z2 = Z02 - (radius2 * TanLambda2 * Phi02Intersect);
      std::pair<double, double>  z1z2(z1, z2);
      return z1z2;
    }

    double convertedPhotonDelZ(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      std::pair<double, double> z1z2 = convertedPhotonZ1Z2(gamma, daughterIndices);
      double z1 = z1z2.first; double z2 = z1z2.second;
      return (z1 - z2);
    }

    double convertedPhotonZ(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      std::pair<double, double> z1z2 = convertedPhotonZ1Z2(gamma, daughterIndices);
      double z1 = z1z2.first; double z2 = z1z2.second;
      return (z1 + z2) * 0.5;
    }

    TVector2 convertedPhotonXY(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      //Do basic checks
      int errFlag = convertedPhotonErrorChecks(gamma, daughterIndices);
      if (errFlag == -1) {return TVector2(Const::doubleNaN, Const::doubleNaN);}

      //Load helix parameters
      double Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02, Omega2, Z02, TanLambda2;
      int daughterIndex1 = int(daughterIndices[0]);
      int daughterIndex2 = int(daughterIndices[1]);
      errFlag = convertedPhotonLoadHelixParams(gamma, daughterIndex1, daughterIndex2, Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02,
                                               Omega2, Z02, TanLambda2);
      if (errFlag == -1) {
        B2ERROR("First track provided has curvature zero. Calculation of convertedPhotonXY failed.");
        return  TVector2(Const::doubleNaN, Const::doubleNaN);
      }
      if (errFlag == -2) {
        B2ERROR("Second track provided has curvature zero. Calculation of convertedPhotonXY failed.");
        return  TVector2(Const::doubleNaN, Const::doubleNaN);
      }

      //Radial unit vectors
      double radius1 = 1 / Omega1;
      double radius2 = 1 / Omega2;

      TVector2 center1((radius1 + D01) * sin(Phi01), -1 * (radius1 + D01) * cos(Phi01));
      TVector2 center2((radius2 + D02) * sin(Phi02), -1 * (radius2 + D02) * cos(Phi02));
      TVector2 cenDiff = center2 - center1;
      double delR = fabs(radius1) + fabs(radius2) - cenDiff.Mod();

      //Calculate transverse vertex
      TVector2 n1 = cenDiff.Unit();
      TVector2 vtxXY = center1 + ((fabs(radius1) - (delR / 2)) * n1);
      return vtxXY;
    }

    double convertedPhotonX(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      auto vtxXY = convertedPhotonXY(gamma, daughterIndices);
      return vtxXY.X();
    }

    double convertedPhotonY(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      auto vtxXY = convertedPhotonXY(gamma, daughterIndices);
      return vtxXY.Y();
    }

    double convertedPhotonRho(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      auto vtxXY = convertedPhotonXY(gamma, daughterIndices);
      return vtxXY.Mod();
    }

    B2Vector3D convertedPhoton3Momentum(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      //Do basic checks
      int errFlag = convertedPhotonErrorChecks(gamma, daughterIndices);
      if (errFlag == -1) {return B2Vector3D(Const::doubleNaN, Const::doubleNaN, Const::doubleNaN);}

      //Load helix parameters
      double Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02, Omega2, Z02, TanLambda2;
      int daughterIndex1 = int(daughterIndices[0]);
      int daughterIndex2 = int(daughterIndices[1]);
      errFlag = convertedPhotonLoadHelixParams(gamma, daughterIndex1, daughterIndex2, Phi01, D01, Omega1, Z01, TanLambda1, Phi02, D02,
                                               Omega2, Z02,
                                               TanLambda2);
      if (errFlag == -1) {
        B2ERROR("First track provided has curvature zero. Calculation of convertedPhoton3Momentum failed.");
        return  B2Vector3D(Const::doubleNaN, Const::doubleNaN, Const::doubleNaN);
      }
      if (errFlag == -2) {
        B2ERROR("Second track provided has curvature zero. Calculation of convertedPhoton3Momentum failed.");
        return  B2Vector3D(Const::doubleNaN, Const::doubleNaN, Const::doubleNaN);
      }

      //Delta-Z
      //Radial unit vectors
      double radius1 = 1 / Omega1;
      double radius2 = 1 / Omega2;

      TVector2 center1((radius1 + D01) * sin(Phi01), -1 * (radius1 + D01) * cos(Phi01));
      TVector2 center2((radius2 + D02) * sin(Phi02), -1 * (radius2 + D02) * cos(Phi02));
      TVector2 n1 =  center1 - center2; n1 = n1.Unit();
      TVector2 n2 = -1 * n1;
      n1 = copysign(1.0, Omega1) * n1;
      n2 = copysign(1.0, Omega2) * n2;

      //Getting running parameter phi at nominal vertex
      double phiN1 = atan2(n1.X(), -n1.Y());
      double phiN2 = atan2(n2.X(), -n2.Y());

      //Sine and cosine Lambda
      double sinlam1 = TanLambda1 / sqrt(1 + (TanLambda1 * TanLambda1));
      double coslam1 = 1 / sqrt(1 + (TanLambda1 * TanLambda1));
      double sinlam2 = TanLambda2 / sqrt(1 + (TanLambda2 * TanLambda2));
      double coslam2 = 1 / sqrt(1 + (TanLambda2 * TanLambda2));

      //Photon 3-momentum
      double p1  = gamma->getDaughter(daughterIndex1)->getMomentumMagnitude();
      B2Vector3D e1Momentum(coslam1 * cos(phiN1), coslam1 * sin(phiN1), sinlam1);
      double p2  = gamma->getDaughter(daughterIndex2)->getMomentumMagnitude();
      B2Vector3D e2Momentum(coslam2 * cos(phiN2), coslam2 * sin(phiN2), sinlam2);
      B2Vector3D gammaMomentum = (e1Momentum * p1) + (e2Momentum * p2);

      return gammaMomentum;
    }

    double convertedPhotonPx(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      auto gammaMomentum = convertedPhoton3Momentum(gamma, daughterIndices);
      return gammaMomentum.Px();
    }

    double convertedPhotonPy(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      auto gammaMomentum = convertedPhoton3Momentum(gamma, daughterIndices);
      return gammaMomentum.Py();
    }

    double convertedPhotonPz(const Particle* gamma, const std::vector<double>& daughterIndices)
    {
      auto gammaMomentum = convertedPhoton3Momentum(gamma, daughterIndices);
      return gammaMomentum.Pz();
    }

    int v0DaughtersShareInnermostHit(const Particle* part)
    {
      if (!part)
        return -1;
      auto daughterPlus  = part->getDaughter(0);
      auto daughterMinus = part->getDaughter(1);
      if (!daughterPlus || !daughterMinus)
        return -1;
      auto trackFitPlus  = daughterPlus->getTrackFitResult();
      auto trackFitMinus = daughterMinus->getTrackFitResult();
      if (!trackFitPlus || !trackFitMinus)
        return -1;
      int flagPlus  = trackFitPlus->getHitPatternVXD().getInnermostHitShareStatus();
      int flagMinus = trackFitMinus->getHitPatternVXD().getInnermostHitShareStatus();
      if (flagPlus != flagMinus)
        return -1;
      return flagPlus;
    }

    bool v0DaughtersShareInnermostUHit(const Particle* part)
    {
      return ((v0DaughtersShareInnermostHit(part) / 2) == 1);
    }

    bool v0DaughtersShareInnermostVHit(const Particle* part)
    {
      return ((v0DaughtersShareInnermostHit(part) % 2) == 1);
    }

    VARIABLE_GROUP("V0Daughter");

    REGISTER_VARIABLE("v0DaughterNCDCHits(i)", v0DaughterTrackNCDCHits, "Number of CDC hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNCDCHits", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nCDCHits`,
                     so replace the current call with ``daughter(i, nCDCHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterNSVDHits(i)", v0DaughterTrackNSVDHits, "Number of SVD hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNSVDHits", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nSVDHits`,
                     so replace the current call with ``daughter(i, nSVDHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterNPXDHits(i)", v0DaughterTrackNPXDHits, "Number of PXD hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNPXDHits", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nPXDHits`,
                     so replace the current call with ``daughter(i, nPXDHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterNVXDHits(i)", v0DaughterTrackNVXDHits, "Number of PXD+SVD hits associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterNVXDHits", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `nVXDHits`,
                     so replace the current call with ``daughter(i, nVXDHits)``.)DOC");
    REGISTER_VARIABLE("v0DaughterNRemovedHits(i)", v0DaughterTrackNRemovedHits,
                      "The number of the i-th daughter track hits removed in V0Finder. Returns 0 if called for something other than V0 daughters.");
    REGISTER_VARIABLE("v0DaughterFirstSVDLayer(i)", v0DaughterTrackFirstSVDLayer,
                      "First activated SVD layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterFirstSVDLayer", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `firstSVDLayer`,
                     so replace the current call with ``daughter(i, firstSVDLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterFirstPXDLayer(i)", v0DaughterTrackFirstPXDLayer,
                      "First activated PXD layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterFirstPXDLayer", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `firstPXDLayer`,
                     so replace the current call with ``daughter(i, firstPXDLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterFirstCDCLayer(i)", v0DaughterTrackFirstCDCLayer,
                      "First activated CDC layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterFirstCDCLayer", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `firstCDCLayer`,
                     so replace the current call with ``daughter(i, firstCDCLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterLastCDCLayer(i)",  v0DaughterTrackLastCDCLayer,
                      "Last CDC layer associated to the i-th daughter track");
    MAKE_DEPRECATED("v0DaughterLastCDCLayer", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `lastCDCLayer`,
                     so replace the current call with ``daughter(i, lastCDCLayer)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPValue(i)", v0DaughterTrackPValue,
                      "chi2 probalility of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterPValue", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `pValue`,
                     so replace the current call with ``daughter(i, pValue)``.)DOC");
    /// helix parameters
    REGISTER_VARIABLE("v0DaughterD0(i)",        v0DaughterTrackD0,        "d0 of the i-th daughter track fit\n\n", "cm");
    MAKE_DEPRECATED("v0DaughterD0", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `d0`,
                     so replace the current call with ``daughter(i, d0)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPhi0(i)",      v0DaughterTrackPhi0,      "phi0 of the i-th daughter track fit\n\n", "rad");
    MAKE_DEPRECATED("v0DaughterPhi0", false, "light-2104-poseidon", R"DOC(
                 The same value can be calculated with the more generic variable `phi0`,
                 so replace the current call with ``daughter(i, phi0)``.)DOC");
    REGISTER_VARIABLE("v0DaughterOmega(i)",     v0DaughterTrackOmega,     "omega of the i-th daughter track fit\n\n",
                      ":math:`\\text{cm}^{-1}`");
    MAKE_DEPRECATED("v0DaughterOmega", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `omega`,
                     so replace the current call with ``daughter(i, omega)``.)DOC");
    REGISTER_VARIABLE("v0DaughterZ0(i)",        v0DaughterTrackZ0,        "z0 of the i-th daughter track fit\n\n", "cm");
    MAKE_DEPRECATED("v0DaughterZ0", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `z0`,
                     so replace the current call with ``daughter(i, z0)``.)DOC");
    REGISTER_VARIABLE("v0DaughterTanLambda(i)", v0DaughterTrackTanLambda, "tan(lambda) of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterTanLambda", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `tanLambda`,
                     so replace the current call with ``daughter(i, tanLambda)``.)DOC");
    /// error of helix parameters
    REGISTER_VARIABLE("v0DaughterD0Error(i)", v0DaughterTrackD0Error, "d0 error of the i-th daughter track fit\n\n", "cm");
    MAKE_DEPRECATED("v0DaughterD0Error", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `d0Err`,
                     so replace the current call with ``daughter(i, d0Err)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPhi0Error(i)", v0DaughterTrackPhi0Error, "phi0 error of the i-th daughter track fit\n\n", "rad");
    MAKE_DEPRECATED("v0DaughterPhi0Error", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `phi0Err`,
                     so replace the current call with ``daughter(i, phi0Err)``.)DOC");
    REGISTER_VARIABLE("v0DaughterOmegaError(i)", v0DaughterTrackOmegaError, "omega error of the i-th daughter track fit\n\n",
                      ":math:`\\text{cm}^{-1}`");
    MAKE_DEPRECATED("v0DaughterOmegaError", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `omegaErr`,
                     so replace the current call with ``daughter(i, omegaErr)``.)DOC");
    REGISTER_VARIABLE("v0DaughterZ0Error(i)", v0DaughterTrackZ0Error, "z0 error of the i-th daughter track fit\n\n", "cm");
    MAKE_DEPRECATED("v0DaughterZ0Error", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `z0Err`,
                     so replace the current call with ``daughter(i, z0Err)``.)DOC");
    REGISTER_VARIABLE("v0DaughterTanLambdaError(i)", v0DaughterTrackTanLambdaError, "tan(lambda) error of the i-th daughter track fit");
    MAKE_DEPRECATED("v0DaughterTanLambdaError", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `tanLambdaErr`,
                     so replace the current call with ``daughter(i, tanLambdaErr)``.)DOC");

    /// V0 daughter helix parameters with V0 vertex as pivot
    REGISTER_VARIABLE("V0d0(id)", v0DaughterD0,
                      "Return the d0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track.\n\n",
                      "cm");
    REGISTER_VARIABLE("V0Deltad0", v0DaughterD0Diff,
                      "Return the difference between d0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track.\n\n",
                      "cm");
    REGISTER_VARIABLE("V0z0(id)", v0DaughterZ0,
                      "Return the z0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track.\n\n",
                      "cm");
    REGISTER_VARIABLE("V0Deltaz0", v0DaughterZ0Diff,
                      "Return the difference between z0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track.\n\n",
                      "cm");

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
    MAKE_DEPRECATED("v0DaughterD0PullWithOriginAsPivot", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `d0Pull`,
                     so replace the current call with ``daughter(i, d0Pull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterPhi0PullWithOriginAsPivot(i)",     v0DaughterHelixWithOriginAsPivotPhi0Pull,
                      "phi0 pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterPhi0PullWithOriginAsPivot", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `phi0Pull`,
                     so replace the current call with ``daughter(i, phi0Pull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterOmegaPullWithOriginAsPivot(i)",    v0DaughterHelixWithOriginAsPivotOmegaPull,
                      "omega pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterOmegaPullWithOriginAsPivot", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `omegaPull`,
                     so replace the current call with ``daughter(i, omegaPull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterZ0PullWithOriginAsPivot(i)",       v0DaughterHelixWithOriginAsPivotZ0Pull,
                      "z0 pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterZ0PullWithOriginAsPivot", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `z0Pull`,
                     so replace the current call with ``daughter(i, z0Pull)``.)DOC");
    REGISTER_VARIABLE("v0DaughterTanLambdaPullWithOriginAsPivot(i)", v0DaughterHelixWithOriginAsPivotTanLambdaPull,
                      "tan(lambda) pull of the i-th daughter track with the origin as the track pivot");
    MAKE_DEPRECATED("v0DaughterTanLambdaPullWithOriginAsPivot", false, "light-2104-poseidon", R"DOC(
                     The same value can be calculated with the more generic variable `tanLambdaPull`,
                     so replace the current call with ``daughter(i, tanLambdaPull)``.)DOC");
    /// helix parameters and covariance matrix elements
    REGISTER_VARIABLE("v0DaughterTau(i,j)",        v0DaughterTrackParam5AtIPPerigee,
                      "j-th track parameter (at IP perigee) of the i-th daughter track. "
                      "j:  0:d0, 1:phi0, 2:omega, 3:z0, 4:tanLambda\n\n", "cm, rad, :math:`\\text{cm}^{-1}`, cm, unitless");
    REGISTER_VARIABLE("v0DaughterCov(i,j)",        v0DaughterTrackParamCov5x5AtIPPerigee,
                      "j-th element of the 15 covariance matrix elements (at IP perigee) of the i-th daughter track. "
                      "(0,0), (0,1) ... (1,1), (1,2) ... (2,2) ..."
                      "index order is:  0:d0, 1:phi0, 2:omega, 3:z0, 4:tanLambda\n\n", "cm, rad, :math:`\\text{cm}^{-1}`, cm, unitless");
    /// Converted photon variables
    REGISTER_VARIABLE("convertedPhotonInvariantMass(i,j)",       convertedPhotonInvariantMass,
                      "Invariant mass of the i-j daughter system as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "GeV/:math:`\\text{c}^2`");
    REGISTER_VARIABLE("convertedPhotonDelTanLambda(i,j)",       convertedPhotonDelTanLambda,
                      "Discriminating variable Delta-TanLambda calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonDelR(i,j)",       convertedPhotonDelR,
                      "Discriminating variable Delta-R calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "cm");
    REGISTER_VARIABLE("convertedPhotonDelZ(i,j)",       convertedPhotonDelZ,
                      "Discriminating variable Delta-Z calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "cm");
    REGISTER_VARIABLE("convertedPhotonX(i,j)",       convertedPhotonX,
                      "Estimate of vertex X coordinate  calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "cm");
    REGISTER_VARIABLE("convertedPhotonY(i,j)",       convertedPhotonY,
                      "Estimate of vertex Y coordinate  calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "cm");
    REGISTER_VARIABLE("convertedPhotonZ(i,j)",       convertedPhotonZ,
                      "Estimate of vertex Z coordinate  calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "cm");
    REGISTER_VARIABLE("convertedPhotonRho(i,j)",       convertedPhotonRho,
                      "Estimate of vertex Rho  calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "cm");
    REGISTER_VARIABLE("convertedPhotonPx(i,j)", convertedPhotonPx,
                      "Estimate of x-component of photon momentum calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "GeV/c");
    REGISTER_VARIABLE("convertedPhotonPy(i,j)", convertedPhotonPy,
                      "Estimate of y-component of photon momentum calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "GeV/c");
    REGISTER_VARIABLE("convertedPhotonPz(i,j)", convertedPhotonPz,
                      "Estimate of z-component of photon momentum calculated for daughters (i,j) as defined in https://indico.belle2.org/event/3644/contributions/18622/attachments/9401/14443/Photon_vertexin_B2GM.pdf, assuming it's a converted photon\n\n",
                      "GeV/c");
    /// check whether the innermost VXD hits are shared among daoughters
    REGISTER_VARIABLE("v0DaughtersShare1stHit", v0DaughtersShareInnermostHit,
                      "flag for V0 daughters sharing the first(innermost) VXD hit. 0x1(0x2) bit represents V/z(U/r-phi)-hit share.");
    REGISTER_VARIABLE("v0DaughtersShare1stUHit", v0DaughtersShareInnermostUHit,
                      "flag for V0 daughters sharing the first(innermost) VXD U-side hit.");
    REGISTER_VARIABLE("v0DaughtersShare1stVHit", v0DaughtersShareInnermostVHit,
                      "flag for V0 daughters sharing the first(innermost) VXD V-side hit.");
  }
}
