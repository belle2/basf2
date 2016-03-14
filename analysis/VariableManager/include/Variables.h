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
     * return cosine of angle between momentum and vertex vector (vector connecting ip and fitted vertex) of this particle
     */
    double cosAngleBetweenMomentumAndVertexVector(const Particle* part);

    /**
     * return Zdistance of daughter tracks at vertex point
     */
    double VertexZDist(const Particle*);

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
     * return mass (determined from particle's daughter 4-momentum vectors under proton mass assumption)
     */
    double particleInvariantMassLambda(const Particle* part);

    /**
     * return uncertainty of the invariant mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMassError(const Particle* part);

    /**
     * return signed deviation of particle's invariant mass from its nominal mass
     */
    double particleInvariantMassSignificance(const Particle* part);

    /**
     * return signed deviation of particle's invariant mass (determined from particle's daughter 4-momentum vectors) from its nominal mass
     */
    double particleInvariantMassBeforeFitSignificance(const Particle* part);

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
     * return StoreArray index (0-based) of the MDST object from which the Particle was created
     */
    double particleMdstArrayIndex(const Particle* part);

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
     * return 1 if Particle is related to initial MCParticle, 0 if Particle is related to non-initial MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCInitialParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to virtual MCParticle, 0 if Particle is related to non-virtual MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCVirtualParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to FSR MCParticle, 0 if Particle is related to non-FSR MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCFSRParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to Photos MCParticle, 0 if Particle is related to non-Photos MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCPhotosParticle(const Particle* particle);

    /**
     * return 1 if Particle is related to ISR MCParticle, 0 if Particle is related to non-ISR MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCISRParticle(const Particle* particle);

    /**
     * returns the PDG code of the correspondig B0/B0bar when found in RoE, 0 otherwise
     */
    double McFlavorOfTagSide(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL), 0 otherwise
     */
    double isSignal(const Particle* particle);

    /**
     * check the PDG code of a particles MC mother
     */
    double genMotherPDG(const Particle* particle);

    /**
     * check the array index of a particle's MC mother
     */
    double genMotherIndex(const Particle* particle);

    /**
     * check the array index of a particle's related MCParticle
     */
    double genParticleIndex(const Particle* particle);

    /**
     * return PDG code of matched MCParticle
     */
    double particleMCMatchPDGCode(const Particle* particle);

    /**
     * return combination of MCMatching::MCErrorFlags flags.
     */
    double particleMCErrors(const Particle* particle);

    /**
     * return the weight of the Particle -> MCParticle relation (only for the first Relation = largest weight)
     */
    double particleMCMatchWeight(const Particle* particle);

    /**
     * return the number of relations of this Particle to MCParticle
     */
    double particleNumberOfMCMatch(const Particle* particle);

    /**
     * return 1 if Particle is related to primary MCParticle, 0 if Particle is related to non-primary MCParticle, -1 if Particle is not related to MCParticle
     */
    double particleMCPrimaryParticle(const Particle* particle);

    /**
     * return 1 if Particle was in RestOfEventObject, 0 otherwise
     */
    double isInRestOfEvent(const Particle* particle);

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
    double nROEECLClusters(const Particle* particle);

    /**
     * return number of remaining KLM clusters as given by the related RestOfEvent object
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double pionVeto(const Particle* particle);

    /**
     * returns 1 if the invariant mass of a combination of a photon in RestOfEvent with
     * the signal photon yields the mass of the a neutral Pion.
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */

    double nROEKLMClusters(const Particle* particle);
    /**
     * return X component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVx(const Particle* particle);

    /**
     * return Y component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVy(const Particle* particle);

    /**
     * return Z component of the tag vertex
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleTagVz(const Particle* particle);

    /**
     * return Delta T (Brec - Btag) in ps
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleDeltaT(const Particle* particle);

    /**
     * return generated Delta T (Brec - Btag) in ps
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleMCDeltaT(const Particle* particle);

    /**
     * return Delta Z (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleDeltaZ(const Particle* particle);

    /**
     * return Delta Boost direction (Brec - Btag) in cm
     *
     * requires that Vertex <-> Particle relation exists (returns -1111 if it doesn't)
     */
    double particleDeltaB(const Particle* particle);

    /**
     * return magnitude of 3-momentum recoiling against given Particle
     */
    double recoilMomentum(const Particle* particle);

    /**
     * returns 1.0 if the particle has been selected as target in the muon or electron flavor tagging category, 0.0 else.
     */
    double isInElectronOrMuonCat(const Particle* particle);

    /**
     * returns cosine of angle between kaon and slow pion momenta, i.e. between the momenta of the particles selected as target kaon and slow pion.
     */
    double cosKaonPion(const Particle* particle);

    /**
     * returns the impact parameter D of the given particle in the xy plane
     */
    double ImpactXY(const Particle* particle);

    /**
     * returns 1 if the particles selected as target kaon and slow pion in the respective flavour tagging categories have oposite charges, 0 else.
     */
    double KaonPionHaveOpositeCharges(const Particle* particle);

    /**
     * returns information regarding the charm quark presence in the decay
     *
     * hasCharmoniumDaughter returns 1 if there is a b->c anti-c q or an anti-b->anti-c c anti-q transition present, 0 otherwise
     */
    double hasCharmoniumDaughter(const Particle* particle);

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
     * return extra energy in the calorimeter that is not associated to the given Particle
     * ECLClusters passing goodGamma selection are used only.
     *
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     */
    double extraEnergyFromGoodGamma(const Particle* particle);

    /**
     * return 1/2/3 if the ECL Cluster is detected in the forward/barrel/backward region
     */
    double eclClusterDetectionRegion(const Particle* particle);

    /** Returns true if the cluster with given attributes passes 'good gamma' criteria.
     *
     * @param calibrated set to false for goodGammaUncalibrated().
     */
    bool isGoodGamma(int region, double energy, bool calibrated);

    /**
     * Return 1 if ECLCluster passes the following selection criteria:
     * Forward  : E > 100 MeV
     * Barrel   : E >  90 MeV
     * Backward : E > 160 MeV
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

