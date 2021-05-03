/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2021 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kota Nakagiri, Yuma Uematsu, Varghese Babu               *
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
#include <mdst/dataobjects/Track.h>
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

    int convertedPhoton_errorChecks(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      //Check that exactly two daugher are indices provided
      if (daughter_indexes.size() != 2) {
        B2ERROR("Invalid number of daughter indices. Please specify exactly two valid daughter indices.");
        return -1;
      }

      //Check that there are atleast (r+1) daughters where r is the bigger of the two indices provided
      int daughter_index1 = int(daughter_indexes[0]);
      int daughter_index2 = int(daughter_indexes[1]);
      if (gamma->getNDaughters() <= std::max(daughter_index1, daughter_index2)) {
        B2ERROR("Invalid daughter indices provided. Particle does not have that many daughters.");
        return -1;
      }

      //Check that there exists tracks associated with the daughter indices provided
      if (!gamma->getDaughter(daughter_index1)->getTrack()) {
        B2ERROR("There is no track associated with daughter index " << daughter_index1);
        return -1;
      }
      if (!gamma->getDaughter(daughter_index2)->getTrack()) {
        B2ERROR("There is no track associated with daughter index " << daughter_index2);
        return -1;
      }

      return 0;
    }


    void convertedPhoton_loadHelixParams(const Particle* gamma, int daughter_index1, int daughter_index2, double& Phi0_1, double& D0_1,
                                         double& Omega_1, double& Z0_1, double& TanLambda_1, double& Phi0_2, double& D0_2, double& Omega_2, double& Z0_2,
                                         double& TanLambda_2)
    {
      //Get helix parameters
      //Electron/track 1
      Helix e1_helix = gamma->getDaughter(daughter_index1)->getTrack()->getTrackFitResultWithClosestMass(Const::electron)->getHelix();

      Phi0_1 = e1_helix.getPhi0();
      D0_1  = e1_helix.getD0() ;
      Omega_1  = e1_helix.getOmega();
      Z0_1   = e1_helix.getZ0();
      TanLambda_1 = e1_helix.getTanLambda();

      //Electron/track 2
      Helix e2_helix = gamma->getDaughter(daughter_index2)->getTrack()->getTrackFitResultWithClosestMass(Const::electron)->getHelix();

      Phi0_2 = e2_helix.getPhi0();
      D0_2  = e2_helix.getD0() ;
      Omega_2  = e2_helix.getOmega();
      Z0_2   = e2_helix.getZ0();
      TanLambda_2 = e2_helix.getTanLambda();
    }

    double convertedPhotonInvariantMass(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      //Do basic checks
      int err_flag = convertedPhoton_errorChecks(gamma, daughter_indexes);
      if (err_flag == -1) {return -999;}

      //Load helix parameters
      double Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2, Omega_2, Z0_2, TanLambda_2;
      int daughter_index1 = int(daughter_indexes[0]);
      int daughter_index2 = int(daughter_indexes[1]);
      convertedPhoton_loadHelixParams(gamma, daughter_index1, daughter_index2, Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2,
                                      Omega_2, Z0_2, TanLambda_2);

      //Calculating invariant mass
      //Sine and cosine Lambda
      double sinlam1 = TanLambda_1 / sqrt(1 + (TanLambda_1 * TanLambda_1));
      double coslam1 = 1 / sqrt(1 + (TanLambda_1 * TanLambda_1));
      double sinlam2 = TanLambda_2 / sqrt(1 + (TanLambda_2 * TanLambda_2));
      double coslam2 = 1 / sqrt(1 + (TanLambda_2 * TanLambda_2));

      //Transverse and longitudinal momentum components; energy with electron mass hypothesis : 0.000511 GeV
      //electron 1
      double p1  = gamma->getDaughter(daughter_index1)->getMomentumMagnitude();
      double pt1 = p1 * coslam1, pz1 =  p1 * sinlam1;
      double e1 = sqrt(pow(p1, 2.0) + pow(0.000511, 2.0));
      //electron 2
      double p2  = gamma->getDaughter(daughter_index2)->getMomentumMagnitude();
      double pt2 = p2 * coslam2, pz2 =  p2 * sinlam2;
      double e2 = sqrt(pow(p2, 2.0) + pow(0.000511, 2.0));

      //Invariant mass of the two track system
      double vtx_mass = sqrt(pow(e1 + e2, 2.0) - pow(pt1 + pt2, 2.0)  - pow(pz1 + pz2, 2.0));
      return vtx_mass;
    }

    double convertedPhotonDelTanLambda(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      //Do basic checks
      int err_flag = convertedPhoton_errorChecks(gamma, daughter_indexes);
      if (err_flag == -1) {return -999;}

      //Load helix parameters
      double Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2, Omega_2, Z0_2, TanLambda_2;
      int daughter_index1 = int(daughter_indexes[0]);
      int daughter_index2 = int(daughter_indexes[1]);
      convertedPhoton_loadHelixParams(gamma, daughter_index1, daughter_index2, Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2,
                                      Omega_2, Z0_2, TanLambda_2);

      //Delta-TanLambda
      return (TanLambda_2 - TanLambda_1);
    }

    double convertedPhotonDelR(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      //Do basic checks
      int err_flag = convertedPhoton_errorChecks(gamma, daughter_indexes);
      if (err_flag == -1) {return -999;}

      //Load helix parameters
      double Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2, Omega_2, Z0_2, TanLambda_2;
      int daughter_index1 = int(daughter_indexes[0]);
      int daughter_index2 = int(daughter_indexes[1]);
      convertedPhoton_loadHelixParams(gamma, daughter_index1, daughter_index2, Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2,
                                      Omega_2, Z0_2, TanLambda_2);

      //Delta-R
      double radius_1 = 1 / Omega_1;
      double radius_2 = 1 / Omega_2;

      TVector2 center1((radius_1 + D0_1) * sin(Phi0_1) , -1 * (radius_1 + D0_1) * cos(Phi0_1));
      TVector2 center2((radius_2 + D0_2) * sin(Phi0_2) , -1 * (radius_2 + D0_2) * cos(Phi0_2));
      TVector2 cen_diff = center1 - center2;

      double del_R = fabs(radius_1) + fabs(radius_2) - cen_diff.Mod();
      return del_R;
    }

    TVector2 convertedPhotonZ1Z2(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      //Do basic checks
      int err_flag = convertedPhoton_errorChecks(gamma, daughter_indexes);
      if (err_flag == -1) {return TVector2(-9999.0, -9999.0);}

      //Load helix parameters
      double Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2, Omega_2, Z0_2, TanLambda_2;
      int daughter_index1 = int(daughter_indexes[0]);
      int daughter_index2 = int(daughter_indexes[1]);
      convertedPhoton_loadHelixParams(gamma, daughter_index1, daughter_index2, Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2,
                                      Omega_2, Z0_2, TanLambda_2);

      //Delta-Z
      //Radial unit vectors
      double radius_1 = 1 / Omega_1;
      double radius_2 = 1 / Omega_2;

      TVector2 center1((radius_1 + D0_1) * sin(Phi0_1) , -1 * (radius_1 + D0_1) * cos(Phi0_1));
      TVector2 center2((radius_2 + D0_2) * sin(Phi0_2) , -1 * (radius_2 + D0_2) * cos(Phi0_2));

      TVector2 n1 =  center1 - center2; n1 = n1.Unit();
      TVector2 n2 = -1 * n1;
      n1 = copysign(1.0, Omega_1) * n1;
      n2 = copysign(1.0, Omega_2) * n2;

      //Getting running parameter phi at nominal vetex
      double phi_n1 = atan2(n1.X(), -n1.Y());
      double phi_n2 = atan2(n2.X(), -n2.Y());
      double Phi0_1_intersect = phi_n1 - Phi0_1;
      double Phi0_2_intersect = phi_n2 - Phi0_2;

      double z1 = Z0_1 - (radius_1 * TanLambda_1 * Phi0_1_intersect);
      double z2 = Z0_2 - (radius_2 * TanLambda_2 * Phi0_2_intersect);
      TVector2 z1_z2(z1, z2);
      return z1_z2;
    }

    double convertedPhotonDelZ(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      TVector2 z1_z2 = convertedPhotonZ1Z2(gamma, daughter_indexes);
      double z1 = z1_z2.X(); double z2 = z1_z2.Y();
      return (z1 - z2);
    }

    double convertedPhotonZ(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      TVector2 z1_z2 = convertedPhotonZ1Z2(gamma, daughter_indexes);
      double z1 = z1_z2.X(); double z2 = z1_z2.Y();
      return (z1 + z2) * 0.5;
    }

    TVector2 convertedPhotonXY(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      //Do basic checks
      int err_flag = convertedPhoton_errorChecks(gamma, daughter_indexes);
      if (err_flag == -1) {return TVector2(-9999.0, -9999.0);}

      //Load helix parameters
      double Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2, Omega_2, Z0_2, TanLambda_2;
      int daughter_index1 = int(daughter_indexes[0]);
      int daughter_index2 = int(daughter_indexes[1]);
      convertedPhoton_loadHelixParams(gamma, daughter_index1, daughter_index2, Phi0_1, D0_1, Omega_1, Z0_1, TanLambda_1, Phi0_2, D0_2,
                                      Omega_2, Z0_2, TanLambda_2);

      //Radial unit vectors
      double radius_1 = 1 / Omega_1;
      double radius_2 = 1 / Omega_2;

      TVector2 center1((radius_1 + D0_1) * sin(Phi0_1) , -1 * (radius_1 + D0_1) * cos(Phi0_1));
      TVector2 center2((radius_2 + D0_2) * sin(Phi0_2) , -1 * (radius_2 + D0_2) * cos(Phi0_2));
      TVector2 cen_diff = center2 - center1;
      double del_R = fabs(radius_1) + fabs(radius_2) - cen_diff.Mod();

      //Calculate transverse vertex
      TVector2 n1 = cen_diff.Unit();
      TVector2 vtx_xy = center1 + ((fabs(radius_1) - (del_R / 2)) * n1);
      return vtx_xy;
    }

    double convertedPhotonX(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      auto vtx_xy = convertedPhotonXY(gamma, daughter_indexes);
      return vtx_xy.X();
    }

    double convertedPhotonY(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      auto vtx_xy = convertedPhotonXY(gamma, daughter_indexes);
      return vtx_xy.Y();
    }

    double convertedPhotonRho(const Particle* gamma, const std::vector<double>& daughter_indexes)
    {
      auto vtx_xy = convertedPhotonXY(gamma, daughter_indexes);
      return vtx_xy.Mod();
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
    /// Converted photon variables
    REGISTER_VARIABLE("convertedPhotonInvariantMass(i,j)",       convertedPhotonInvariantMass,
                      "Invariant mass of the i-j daughter system assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonDelTanLambda(i,j)",       convertedPhotonDelTanLambda,
                      "Discriminating variable Delta-TanLambda calculated for daughters (i,j), assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonDelR(i,j)",       convertedPhotonDelR,
                      "Discriminating variable Delta-R calculated for daughters (i,j), assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonDelZ(i,j)",       convertedPhotonDelZ,
                      "Discriminating variable Delta-Z calculated for daughters (i,j), assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonX(i,j)",       convertedPhotonX,
                      "Estimate of vertex X coordinate  calculated for daughters (i,j), assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonY(i,j)",       convertedPhotonY,
                      "Estimate of vertex Y coordinate  calculated for daughters (i,j), assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonZ(i,j)",       convertedPhotonZ,
                      "Estimate of vertex Z coordinate  calculated for daughters (i,j), assuming it's a converted photon");
    REGISTER_VARIABLE("convertedPhotonRho(i,j)",       convertedPhotonRho,
                      "Estimate of vertex Rho  calculated for daughters (i,j), assuming it's a converted photon");

  }
}
