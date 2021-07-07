/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <mdst/dataobjects/MCParticle.h>

#include <TLorentzVector.h>

#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {

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
     * return 1 if Particle is correctly reconstructed (SIGNAL), 0 otherwise
     * It behaves according to DecayStringGrammar.
     */
    double isSignal(const Particle* part);

    /**
     * return 1 if Particle is almost correctly reconstructed (SIGNAL), 0 otherwise.
     * Misidentification of charged FSP is allowed.
     */
    double isSignalAcceptWrongFSPs(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL including missing neutrino), 0 otherwise
     */
    double isSignalAcceptMissingNeutrino(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL including missing massive), 0 otherwise
     */
    double isSignalAcceptMissingMassive(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL including missing gamma), 0 otherwise
     */
    double isSignalAcceptMissingGamma(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL including bremsstrahlung photons), 0 otherwise
     */
    double isSignalAcceptBremsPhotons(const Particle* part);

    /**
     * return 1 if Particle is correctly reconstructed (SIGNAL including missing all particles), 0 otherwise
     */
    double isSignalAcceptMissing(const Particle* part);

    /**
     * return 1 if the charge of the particle is wrong. 0 in
     * all other cases
     */
    double isWrongCharge(const Particle* particle);

    /**
     * Return 1 if the charged final state particle comes from a
     * cloned track, 0 if not a clone or not from a track, and
     * returns NAN if MCParticle not found (like for data or if
     * not MCMatched)");
     */
    double isCloneTrack(const Particle* particle);

    /**
     * Return 1 if the particle is a clone track or has a clone
     * track as a daughter, 0 otherwise.
     */
    double isOrHasCloneTrack(const Particle* particle);

    /**
     * return 1 if the particle was misidentified (note that this
     * can occur if the wrong charge is assigned). 0 all other cases
     */
    double isMisidentified(const Particle* particle);

    /**
     * Check the PDG code of a particles n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc.
     */
    double genNthMotherPDG(const Particle* part, const std::vector<double>& daughterIDs);

    /**
     * Check the array index of a particle n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc.
     */
    double genNthMotherIndex(const Particle* part, const std::vector<double>& daughterIDs);

    /**
     * check the PDG code of a particles MC mother
     */
    double genMotherPDG(const Particle* particle);

    /**
     * check the array index of a particle's MC mother
     */
    double genMotherIndex(const Particle* particle);

    /**
     * generated momentum of a particles MC mother
     */
    double genMotherP(const Particle* particle);

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

    /*
     * return the true momentum transfer to lepton pair in a B (semi-) leptonic B meson decay
     */
    double particleMCMomentumTransfer2(const Particle* part);

    /**
     * return decay time of matched MCParticle (NaN if the particle is not matched)
     * note this is the delta time between decay of the particle and collision
     */
    double particleMCMatchDecayTime(const Particle* particle);


    /** return life time of matched mc particle in CMS frame. */
    double particleMCMatchLifeTime(const Particle* particle);

    /**
     * return px of matched MCParticle (NaN if the particle is not matched)
     */
    double particleMCMatchPX(const Particle* particle);

    /**
     * return py of matched MCParticle (NaN if the particle is not matched)
     */
    double particleMCMatchPY(const Particle* particle);

    /**
     * return pz of matched MCParticle (NaN if the particle is not matched)
     */
    double particleMCMatchPZ(const Particle* particle);

    /**
     * return pt of matched MCParticle (NaN if the particle is not matched)
     */
    double particleMCMatchPT(const Particle* particle);

    /**
     * return Energy of matched MCParticle (NaN if the particle is not matched)
     */
    double particleMCMatchE(const Particle* particle);

    /**
     * return total momentum of matched MCParticle (NaN if the particle is not matched)
     */
    double particleMCMatchP(const Particle* particle);

    /**
     * Helper function: return total 4-momentum of all daughter neutrinos, recursively down decay tree
     */
    TLorentzVector MCInvisibleP4(const MCParticle* mcparticle);

    /**
     * return cosThetaBetweenParticleAndNominalB using B momentum with all (grand^n)daughter neutrino momenta subtracted
     */
    double particleMCCosThetaBetweenParticleAndNominalB(const Particle* particle);

    /**
     * return recoiling mass against the particles appended as particle's daughters.
     * MC truth values are used in the calculations.
     */
    double particleMCRecoilMass(const Particle* particle);

    /**
     * return the weight produced by the event generator
     */
    double generatorEventWeight(const Particle*);

    /**
     * return the ID of the generated decay of positive tau in a tau pair event.
     */
    int tauPlusMcMode(const Particle*);

    /**
     * return the ID of the generated decay of negative tau in a tau pair event.
     */
    int tauMinusMcMode(const Particle*);

    /**
     * return the prong of the generated decay of positive tau in a tau pair event.
     */
    int tauPlusMcProng(const Particle*);

    /**
     * return the prong of the generated decay of negative tau in a tau pair event.
     */
    int tauMinusMcProng(const Particle*);

    /** check that neutrals were seen in ECL, and charged were seen in SVD or VTX */
    double isReconstructible(const Particle*);

    /** the particle was seen in the PXD */
    double seenInPXD(const Particle*);

    /** the particle was seen in the SVD */
    double seenInSVD(const Particle*);

    /** the particle was seen in the VTX */
    double seenInVTX(const Particle*);

    /** the particle was seen in the CDC */
    double seenInCDC(const Particle*);

    /** the particle was seen in the TOP */
    double seenInTOP(const Particle*);

    /** the particle was seen in the ECL */
    double seenInECL(const Particle*);

    /** the particle was seen in the ARICH */
    double seenInARICH(const Particle*);

    /** the particle was seen in the KLM */
    double seenInKLM(const Particle*);

    /**
     * return number of steps to i-th daughter from the particle at generator level.
     * return NaN, if no MCParticle is associated to the particle or the i-th daughter.
     * return NaN, if i-th daughter does not exist.
     */
    int genNStepsToDaughter(const Particle* particle, const std::vector<double>& arguments);

    /**
     * return number of missing daughters having assigned PDG codes
     * return NaN, if the particle does not have related MC Particle
     */
    int genNMissingDaughter(const Particle* particle, const std::vector<double>& arguments);

    /**
     * return energy of HER from generator level beam kinematics
     */
    double getHEREnergy(const Particle*);

    /**
     * return energy of LER from generator level beam kinematics
     */
    double getLEREnergy(const Particle*);

    /**
     * return crossing angle in the x-z plane from generator level beam kinematics
     */
    double getCrossingAngleX(const Particle*);

    /**
     * return crossing angle in the y-z plane from generator level beam kinematics
     */
    double getCrossingAngleY(const Particle*);

    /**
     * returns the weight of the ECLCluster -> MCParticle relation for the
     * MCParticle related to the particle provided.
     */
    double particleClusterMatchWeight(const Particle* particle);

    /**
     * returns the weight of the ECLCluster -> MCParticle relation for the
     * relation with the largest weight.
     */
    double particleClusterBestMCMatchWeight(const Particle*);

    /**
     * returns the PDG code of the MCParticle for the ECLCluster -> MCParticle
     * relation with the largest weight. For particles created from ECLClusters
     * (photons) this will be the same as the mcPDG, for electrons or other
     * track based particles, the best mc match for the ECLCluster but might not
     * correspond to the track's mcmatch (== the particle)
     */
    double particleClusterBestMCPDGCode(const Particle*);

    /**
     * returns 1 for crossfeed in reconstruction of a B meson, 0 for no crossfeed and
     * nan for no true B meson or failed truthmatching. Iterates over final state daughters
     * of a given B meson and searches for common identical B meson ancestor at generator level.
     */
    double isBBCrossfeed(const Particle*);
  }
}

