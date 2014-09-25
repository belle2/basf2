/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * return momentum magnitude
     */
    double particleP(const Particle* part);

    /**
     * return energy
     */
    double particleE(const Particle* part);

    /**
     * return momentum component x
     */
    double particlePx(const Particle* part);

    /**
     * return momentum component y
     */
    double particlePy(const Particle* part);

    /**
     * return momentum component z
     */
    double particlePz(const Particle* part);

    /**
     * return transverse momentum
     */
    double particlePt(const Particle* part);

    /**
     * return  momentum cosine of polar angle
     */
    double particleCosTheta(const Particle* part);

    /**
     * return momentum azimuthal angle
     */
    double particlePhi(const Particle* part);

    /**
     * return momentum magnitude in CMS
     */
    double particleP_CMS(const Particle* part);

    /**
     * return energy in CMS
     */
    double particleE_CMS(const Particle* part);

    /**
     * return momentum component x in CMS
     */
    double particlePx_CMS(const Particle* part);

    /**
     * return momentum component y in CMS
     */
    double particlePy_CMS(const Particle* part);

    /**
     * return momentum component z in CMS
     */
    double particlePz_CMS(const Particle* part);

    /**
     * return transverse momentum in CMS
     */
    double particlePt_CMS(const Particle* part);

    /**
     * return momentum cosine of polar angle in CMS
     */
    double particleCosTheta_CMS(const Particle* part);

    /**
     * return momentum azimuthal angle in CMS
     */
    double particlePhi_CMS(const Particle* part);

    /**
     * return cosine of angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle
     */
    double cosAngleBetweenMomentumAndVertexVector(const Particle* part);

    /**
     * return distance relative to interaction point
     */
    double particleDistance(const Particle* part);

    /**
     * return significance of distance relative to interaction point
     * (distance relative to interaction point)/ ( error on distance measurement )
     */
    double particleDistanceSignificance(const Particle* part);

    /**
     * return position in x relative to interaction point
     */
    double particleDX(const Particle* part);

    /**
     * return position in y relative to interaction point
     */
    double particleDY(const Particle* part);

    /**
     * return position in z relative to interaction point
     */
    double particleDZ(const Particle* part);

    /**
     * return transverse distance relative to interaction point
     */
    double particleDRho(const Particle* part);

    /**
     * return mass (determined from particle's 4-momentum vector)
     */
    double particleMass(const Particle* part);

    /**
     * return mass minus nominal mass
     */
    double particleDMass(const Particle* part);

    /**
     * return mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMass(const Particle* part);

    /**
     * return uncertainty of the invariant mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMassError(const Particle* part);

    /**
     * return signed deviation of particle's invariant mass from its nominal mass
     */
    double particleInvariantMassSignificance(const Particle* part);

    /**
     * return released energy in decay
     */
    double particleQ(const Particle* part);

    /**
     * return released energy in decay minus nominal one
     */
    double particleDQ(const Particle* part);

    /**
     * return beam constrained mass
     */
    double particleMbc(const Particle* part);

    /**
     * return energy difference in CMS
     */
    double particleDeltaE(const Particle* part);

    /**
     * return prob(chi^2,ndf) of fit
     */
    double particlePvalue(const Particle* part);

    /**
     * return number of daughter particles
     */
    double particleNDaughters(const Particle* part);

    /**
     * return flavor type
     */
    double particleFlavorType(const Particle* part);

    /**
       * return charge
       */
    double particleCharge(const Particle* part);

    /**
       * return cosine of angle between thrust axis of given particle and thrust axis of ROE
       */
    double cosTPTO(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL), 0 otherwise
     */
    double isSignal(const Particle* particle);

    /**
     * return absolute PDG code of matched MCParticle
     */
    double particleAbsMCMatchPDGCode(const Particle* particle);

    /**
     * return PDG code of matched MCParticle
     */
    double particleMCMatchPDGCode(const Particle* particle);

    /**
     * return ORed combination of MCMatching::MCMatchStatus flags.
     */
    double particleMCMatchStatus(const Particle* particle);

    /**
     * return 1 if Particle was in RestOfEventObject, 0 otherwise
     */
    double isInRestOfEvent(const Particle* particle);

    /**
     * return 1 if (dummy)
     */
    double isMajorityInRestOfEventFromB0(const Particle*);

    /**
     * return 1 if (dummy)
     */
    double isMajorityInRestOfEventFromB0bar(const Particle*);

    /**
     * return 1 if (dummy)
     */
    double isRestOfEventOfB0(const Particle*);

    /**
     * return 1 if (dummy)
     */
    double isRestOfEventOfB0bar(const Particle*);

    /**
     * -1 (1) if current RestOfEvent is related to a B0bar (B0)
     */
    double isRestOfEventB0Flavor(const Particle*);

    /**
     * 0 (1) if current RestOfEvent is related to a B0bar (B0)
     */
    double isRestOfEventB0Flavor_Norm(const Particle*);

    /**
     * returns missing Momentum on the tag side (flavor tagging specific variable).
     *
     * requires that StoreObjPtr<RestOfEvent> roe("RestOfEvent") exists.
     */
    double p_miss(const Particle*);

    /**
     * Returns the number of K_S0 in the remaining Kaon ROE (flavor tagging specific variable).
     *
     * requires that StoreObjPtr<ParticleList> KShorts("K_S0:ROEKaon") exists.
     */
    double NumberOfKShortinRemainingROEKaon(const Particle*);

    /**
     * Returns the number of K_S0 in the remaining Lambda ROE (flavor tagging specific variable).
     *
     * requires that StoreObjPtr<ParticleList> KShorts("K_S0:ROELambda") exists.
     */
    double NumberOfKShortinRemainingROELambda(const Particle*);

    /**
     * 1.0 if pdg-code for Lambda0, -1.0 if Anti-Lambda0, 0.0 else
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double lambdaFlavor(const Particle* particle);

    /**
     * Returns the Matrixelement[2][2] of the PositionErrorMatrix of the Vertex fit.
     *
     * This is a simplistic hack. But I see no other way to get the information.
     */
    double lambdaZError(const Particle* particle);

    /**
     * Returns the Momentum of second daughter if existing, else 0.
     */
    double MomentumOfSecondDaughter(const Particle* part);

    /**
     * Returns the Momentum of second daughter if existing in CMS, else 0.
     */
    double MomentumOfSecondDaughter_CMS(const Particle* part);

    /**
     * Returns q*(highest PID_Likelihood for Kaons), else 0.
     */
    double chargeTimesKaonLiklihood(const Particle*);

    /**
     * Returns the transverse momentum of all charged tracks if there exists a ROE for the given particle, else 0.
     */
    double transverseMomentumOfChargeTracksInRoe(const Particle* part);

    /**
     * return number of remaining tracks as given by the related RestOfEvent object
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double nROETracks(const Particle* particle);

    /**
     * return number of remaining ECL clusters as given by the related RestOfEvent object
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double nROEClusters(const Particle* particle);

    /**
     * Flavour of Btag from trained Method
     */
    double particleClassifiedFlavor(const Particle* particle);

    /**
     * Flavour of Btag from MC
     */
    double particleMCFlavor(const Particle* particle);

    /**
     * return magnitude of 3-momentum recoiling against given Particle
     */
    double recoilMomentum(const Particle* particle);

    /**
     * return energy recoiling against given Particle
     */
    double recoilEnergy(const Particle* particle);

    /**
     * return invariant mass of the system recoiling against given Particle
     */
    double recoilMass(const Particle* particle);

    /**
     *
     * return invarian mass squared of the system recoiling against given Particle
     */
    double recoilMassSquared(const Particle* particle);

    /**
     * return extra energy in the calorimeter that is not associated to the given Particle
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double extraEnergy(const Particle* particle);

    /**
     * return 1/2/3 if the ECL Cluster is detected in the forward/barrel/backward region
     */
    double eclClusterDetectionRegion(const Particle* particle);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E >  85 MeV && E9/E25>0.7
     * Barrel   : E >  60 MeV
     * Backward : E > 110 MeV
     */
    double goodGamma(const Particle* particle);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E_uncalib >  125 MeV && E9/E25>0.7
     * Barrel   : E_uncalib > 100 MeV
     * Backward : E_uncalib > 150 MeV
     */
    double goodGammaUncalibrated(const Particle* particle);

    /**
     * return ECL cluster's uncorrected energy
     */
    double eclClusterUncorrectedE(const Particle* particle);

    /**
     * return ECL cluster's distance
     */
    double eclClusterR(const Particle* particle);

    /**
     * return ECL cluster's azimuthal angle
     */
    double eclClusterPhi(const Particle* particle);

    /**
     * return ECL cluster's polar angle
     */
    double eclClusterTheta(const Particle* particle);

    /**
     * return ECL cluster's timing
     */
    double eclClusterTiming(const Particle* particle);

    /**
     * return the energy of the crystall with highest  energy
     */
    double eclClusterHighestE(const Particle* particle);

    /**
     * return ratio of energies in inner 3x3 and 5x5 cells
     */
    double eclClusterE9E25(const Particle* particle);

    /**
     *
     * return number of hits associated to this cluster
     */
    double eclClusterNHits(const Particle* particle);

    /**
     * return 1/0 if charged track is/is not Matched to this cluster
     */
    double eclClusterTrackMatched(const Particle* particle);


    /**
     * returns true if event doesn't contain an Y(4S)
     */
    double isContinuumEvent(const Particle*);

    /**
     * return number of tracks in event
     */
    double nTracks(const Particle*);

    /**
     * return number of ECL clusters in event
     */
    double nECLClusters(const Particle*);

    /**
     * return number of KLM clusters in event
     */
    double nKLMClusters(const Particle*);

    /**
     * return total energy in ECL clusters in event
     */
    double ECLEnergy(const Particle*);

    /**
     * return total energy in KLM clusters in event
     */
    double KLMEnergy(const Particle*);

    /**
     * return always zero
     */
    double False(const Particle*);

  }
} // Belle2 namespace

