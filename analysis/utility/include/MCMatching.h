#pragma once
// ******************************************************************
// MC Matching
// authors: A. Zupanc (anze.zupanc@ijs.si), C. Pulvermacher (christian.pulvermacher@kit.edu)
// ******************************************************************


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
  namespace MCMatching {
    /** Name of extra-info field stored in Particle. */
    extern const std::string c_extraInfoMCErrors;

    /** Flags that describe different reconstruction errors. */
    enum MCErrorFlags {
      c_Correct             = 0,  /**< This Particle and all its daughters are perfectly reconstructed. */
      c_MissFSR             = 1, /**< A Final State Radiation (FSR) photon is not reconstructed. Note that distinction from c_MissGamma is based only on number of other daughters and may be wrong. */
      c_MissingResonance    = 2, /**< The associated MCParticle decay contained additional non-final-state particles (e.g. a rho) that weren't reconstructed. This is probably O.K. in most cases*/
      c_DecayInFlight       = 4, /**< A Particle was reconstructed from the secondary decay product of the actual particle. This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might mean a pion hypothesis was used for a secondary electron. */
      c_MissNeutrino        = 8, /**< A neutrino is missing (not reconstructed). */
      c_MissGamma           = 16, /**< A photon (not FSR) is missing (not reconstructed). */
      c_MissMassiveParticle = 32, /**< A generated massive FSP is missing (not reconstructed). */
      c_MissKlong           = 64, /**< A Klong is missing (not reconstructed). */
      c_MisID               = 128, /**< One of the charged final state particles is mis-identified. */
      c_AddedWrongParticle  = 256, /**< A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters) belongs to another Particle. */
      c_InternalError       = 512, /**< There was an error in MC matching. Not a valid match. Might indicate fake/background track or cluster. */
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
    std::string explainFlags(unsigned int flags);

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
    bool setMCTruth(const Belle2::Particle* particle);

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
     * @param mc pointer to the matched MCParticle. Can be specified to avoid repeated lookups.
     *
     * @return ORed combination of MCErrorFlags describing differences between reconstructed particle and MC truth.
     */
    int getMCErrors(const Belle2::Particle* particle, const Belle2::MCParticle* mcParticle = nullptr);

    /** Sets error flags in extra-info (also returns it).
     *
     * Users should use getMCErrors(), which only calculates this information when necessary.
     */
    int setMCErrorsExtraInfo(Belle2::Particle* particle, const Belle2::MCParticle* mcParticle);

    /**
     * Fills vector with array (1-based) indices of all generator ancestors of given MCParticle.
     *
     * @param pointer to the MCParticle
     * @param reference to the vector of integers to hold the results
     */
    void fillGenMothers(const Belle2::MCParticle* mcP, std::vector<int>& genMCPMothers);

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
    int findCommonMother(const Belle2::MCParticle* mcP, const std::vector<int>& firstMothers, int lastMother);

    /**
     * Returns true if given PDG code indicates a FSP.
     */
    bool isFSP(int pdg);

    /**
     * Returns true if given MCParticle is a final state radiation (FSR) photon.
     * Assumes PDG code of p is 22.
     *
     * Note: this is a bit rough, needs some changes to EvtGen to determine this reliably.
     */
    bool isFSR(const Belle2::MCParticle* p);

    /**
     * Determines which daughters of 'mcParticle' are not reconstructed by any daughter of
     * 'particle'.
     *
     * @returns ORed combination of MCErrorFlags flags for missing particles.
     */
    int getMissingParticleFlags(const Belle2::Particle* particle, const Belle2::MCParticle* mcParticle);

    /**
     * Determines the number of daughter particles which are not neutrinos.
     * Needed to handle the special case tau -> rho nu correctly.
     * @param mcParticle
     * @return number of daughters which are not neutrinos
     */
    int getNumberOfDaughtersWithoutNeutrinos(const MCParticle* mcParticle);
  }
}
