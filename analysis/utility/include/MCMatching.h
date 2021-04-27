/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 *                                                                        *
 * Copyright(C) 2013-2018 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc, Christian Pulvermacher, Yo Sato             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;
  class MCParticle;

  /** Functions to perform Monte Carlo matching for reconstructed Particles.
   *
   * End users should usually not need to run these functions directly, but can use the matchMCTruth() python function in modularAnalysis (or the MCMatching module). Afterwards, Particles have relations to the matched MCParticle.
   *
   * Different MCErrorFlags flags can be queried using getMCErrors(), or the associated 'mcErrors' variable (available via VariablesToNtuple, the MCTruth ntuple tool, etc.). For checking if a Particle is correctly reconstructed, the 'isSignal' variable can be used.
   */
  struct MCMatching {
    /** Name of extra-info field stored in Particle. */
    static const std::string c_extraInfoMCErrors;

    /** Flags that describe different reconstruction errors. */
    enum MCErrorFlags {
      c_Correct               = 0,  /**< This Particle and all its daughters are perfectly reconstructed. */
      c_MissFSR               = 1, /**< A Final State Radiation (FSR) photon is not reconstructed (based on MCParticle::c_IsFSRPhoton). */
      c_MissingResonance      = 2, /**< The associated MCParticle decay contained additional non-final-state particles (e.g. a rho) that weren't reconstructed. This is probably O.K. in most cases*/
      c_DecayInFlight         = 4, /**< A Particle was reconstructed from the secondary decay product of the actual particle. This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might mean a pion hypothesis was used for a secondary electron. */
      c_MissNeutrino          = 8, /**< A neutrino is missing (not reconstructed). */
      c_MissGamma             = 16, /**< A photon (not FSR) is missing (not reconstructed). */
      c_MissMassiveParticle   = 32, /**< A generated massive FSP is missing (not reconstructed). */
      c_MissKlong             = 64, /**< A Klong is missing (not reconstructed). */
      c_MisID                 = 128, /**< One of the charged final state particles is mis-identified. */
      c_AddedWrongParticle    = 256, /**< A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters) belongs to another Particle. */
      c_InternalError         = 512, /**< There was an error in MC matching. Not a valid match. Might indicate fake/background track or cluster. */
      c_MissPHOTOS            = 1024, /**< A photon created by PHOTOS was not reconstructed (based on MCParticle::c_IsPHOTOSPhoton) */
      c_AddedRecoBremsPhoton  = 2048, /**< A photon added with the bremsstrahlung recovery tools (correctBrems or correctBremsBelle) has no MC particle assigned, or it doesn't belong to the decay chain */
    };

    /** Return string with all human-readable flags, e.g. explainFlags(402) returns "c_MissingResonance | c_MissGamma | c_MisID | c_AddedWrongParticle".
     *
     * Can also be used from Python:
     * \code{.py}
        import basf2
        from ROOT import Belle2, gInterpreter
        gInterpreter.ProcessLine('#include "analysis/utility/MCMatching.h"')
        print(Belle2.MCMatching.explainFlags(123))
       \endcode
     */
    static std::string explainFlags(unsigned int flags);

    /**
     * This is the main function of MC matching algorithm. When executed the algorithm
     * searches for first common generated mother (MCParticle) of this particle's daughters.
     * If such MCParticle is found the Particle -> MCParticle relation is set between them.
     * If the relation between daughter particles and matched generated MC particle does
     * not exist the algorithm sets it also for them.
     *
     * @param particle pointer to the Particle to be mc-matched
     *
     * @return returns true if relation is set and false otherwise
     */
    static bool setMCTruth(const Belle2::Particle* particle);

    /**
     * Returns quality indicator of the match as a bit pattern
     * where the individual bits indicate the the type of mismatch. The values are defined in the
     * MCErrorFlags enum and described in detail there.
     *
     * A return value equal to c_Correct == 0 indicates a perfect MC match (everything OK).
     *
     * The value is also stored inside the Particle's extra-info fields
     * (so it is calculated only once per candidate).
     *
     * @param particle pointer to the particle. setMCTruth() must have been called previously (usually via the MCMatching module)!
     * @param mcParticle pointer to the matched MCParticle. Can be specified to avoid repeated lookups.
     *
     * @return ORed combination of MCErrorFlags describing differences between reconstructed particle and MC truth.
     */
    static int getMCErrors(const Belle2::Particle* particle, const Belle2::MCParticle* mcParticle = nullptr);


    /** Sets error flags in extra-info (also returns it).
     *
     * Users should use getMCErrors(), which only calculates this information when necessary.
     */
    static int setMCErrorsExtraInfo(Belle2::Particle* particle, const Belle2::MCParticle* mcParticle);

    /**
     * Fills vector with array (1-based) indices of all generator ancestors of given MCParticle.
     *
     * @param mcP pointer to the MCParticle
     * @param genMCPMothers pointer to the generator ancestors MCParticle
     * @param reference to the vector of integers to hold the results
     */
    static void fillGenMothers(const Belle2::MCParticle* mcP, std::vector<int>& genMCPMothers);

    /**
     * Finds a mother of mcP that is in firstMothers, from [lastMother,  end]
     *
     * To actually find the common mother of all daughters, each time this function is called for a daughter particle, specify the return value from the last call for lastMother.
     *
     * Note: this looks like an ideal case for applying a more sophisticated algorithm for LCA (lowest common ancestor) problems.
     * However, our trees aren't very large, so additional preprocessing might slow this down (I saw this when experimenting
     * with the algorithm). This might still be worth looking into if an algorithm with inexpensive preprocessing were used,
     * or the preprocessing was done only once per event. See http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=lowestCommonAncestor
     * for an interesting introduction to related algorithms.
     *
     * @return index of the first common mother in firstMothers (!), or -1 if not found.
     */
    static int findCommonMother(const Belle2::MCParticle* mcP, const std::vector<int>& firstMothers, int lastMother);

    /**
     * Returns true if given PDG code indicates a FSP.
     */
    static bool isFSP(int pdg);

    /**
     * Returns true if given MCParticle is a final state radiation (FSR) photon based on MCParticle::c_IsFSRPhoton.
     * Assumes PDG code of p is 22.
     */
    static bool isFSR(const Belle2::MCParticle* p);

    /**
     * Returns true if given MCParticle is a final state radiation (FSR) photon.
     * This is a legacy version of isFSR(const Belle2::MCParticle* p) function that works on MC samples
     * without MCParticle::c_IsFSRPhoton and MCParticle::c_IsPHOTOSPhoton properly set (for example Belle MC).
     */
    static bool isFSRLegacy(const Belle2::MCParticle* p);

    /**
     * Returns true if given MCParticle is a radiative photon.
     * Assumes PDG code of p is 22.
     */
    static bool isRadiativePhoton(const Belle2::MCParticle* p);

    /**
     * Determines which daughters of 'mcParticle' are not reconstructed by any daughter of
     * 'particle'.
     *
     * @returns ORed combination of MCErrorFlags flags for missing particles.
     */
    static int getMissingParticleFlags(const Belle2::Particle* particle, const Belle2::MCParticle* mcParticle);

    /**
     * Returns flags of given Final State Particle.
     *
     * @param particle
     * @param mcParticle
     * @return flags of given particle
     */
    static int getFlagsOfFSP(const Particle* particle, const MCParticle* mcParticle);

    /**
     * Returns flags of daughters of given particle.
     *
     * @param daughter
     * @param mcParticle
     * @return flags of daughters of given particle
     */
    static int getFlagsOfDaughters(const Particle* daughter, const MCParticle* mcParticle);

    /**
     * Returns flags of given daughter which is a brems photon.
     * Special treatment for brems is done.
     * @param daughter
     * @param mcParticle (this is MC mother of daughter)
     * @param genParts vector of MC (n*grand-)daughters
     * @return flags of given daughter
     */
    static int getFlagsOfBremsPhotonDaughter(const Particle* daughter, const MCParticle* mcParticle,
                                             const std::vector<const MCParticle*>& genParts);

    /**
     * Determines the number of daughter particles which are not neutrinos.
     * Needed to handle the special case tau -> rho nu correctly.
     * @param mcParticle
     * @return number of daughters which are not neutrinos
     */
    static int getNumberOfDaughtersWithoutNeutrinos(const MCParticle* mcParticle);

    /**
     * Count the number of missing daughters of the 'particle'.
     * @return number of missing daughters having given PDG codes
     */
    static int countMissingParticle(const Belle2::Particle* particle, const Belle2::MCParticle* mcParticle,
                                    const std::vector<int>& daughterPDG);

    /**
     * Returns the flags ignored by PropertyFlags of given particle.
     * Only c_isIgnored... flags are considered. c_isUnspecified is already considered in setMCErrorsExtraInfo function.
     * @return ORed combination of corresponding MCErrorFlags with PropertyFlags of given particle
     */
    static int getFlagsIgnoredByProperty(const Belle2::Particle* particle);

    /**
     * Returns the daughter mask from given daughterProperty.
     * @return ORed combination of MCErrorFlags to be accepted for the daughter
     */
    static int makeDaughterAcceptMask(int daughterProperty);

  };
}
