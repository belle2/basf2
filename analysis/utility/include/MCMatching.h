#pragma once
// ******************************************************************
// MC Matching
// authors: A. Zupanc (anze.zupanc@ijs.si), C. Pulvermacher (christian.pulvermacher@kit.edu)
// ******************************************************************


#include <vector>

namespace Belle2 {
  class Particle;
  class MCParticle;

  /** Functions to perform Monte Carlo matching for reconstructed Particles.
   *
   * End users should usually not need to run these functions directly, but can use the matchMCTruth() python function in modularAnalysis (or the MCMatching module). Afterwards, Particles have relations to the matched MCParticle.
   *
   * Different MCMatchStatus flags can be queried using getMCTruthStatus(), or the associated 'mcStatus' variable (available to PSelector etc.). For checking if a Particle is correctly reconstructed, the 'isSignal' variable can be used.
   */
  namespace MCMatching {
    /** Flags that describe different reconstruction errors. */
    enum MCMatchStatus {
      c_Correct             = 0,  /**< This Particle and all its daughters are perfectly reconstructed. */
      c_MissFSR             = 1, /**< A Final State Radiation (FSR) photon is not reconstructed. Note that distinction from c_MissGamma is based only on number of other daughters and may be wrong. */
      c_MissingResonance    = 2, /**< The associated MCParticle decay contained additional particles (e.g. a rho) that weren't reconstructed. This is probably O.K. in most cases*/
      c_DecayInFlight       = 4, /**< A Particle was reconstructed from the secondary decay product of the actual particle. This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might mean a pion hypothesis was used for a secondary electron. */
      c_MissNeutrino        = 8, /**< A Neutrino is missing (not reconstructed). */
      c_MissGamma           = 16, /**< A Photon (not FSR) is missing (not reconstructed). */
      c_MissMassiveParticle = 32, /**< A generated massive FSP is missing (not reconstructed). */
      c_MissKlong           = 64, /**< A Klong is missing (not reconstructed). */
      c_MisID               = 128, /**< One of the charged final state particles is mis-identified. */
      c_AddedWrongParticle  = 256, /**< A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters) belongs to another Particle. */
      c_InternalError       = 512, /**< There was an error in MC matching. Not a valid match. */
    };

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
    bool setMCTruth(const Particle* particle);

    /**
     * Returns the status (quality indicator) of the match. The status is given as a bit pattern,
     * where the individual bits indicate the the type of mismatch. The values are defined in the
     * MCMatchStatus enum and described in detail there.
     *
     * Status equal to c_Correct == 0 indicates the perfect MC match (evrything OK).
     * c_InternalError is used to indicate an internal error.
     *
     * The mctruth value is also stored inside the MCTruthStatus extrainfo (so it is calculated only once per candidate).
     *
     * @param particle pointer to the particle
     * @param mc pointer to the matched MCParticle. Can be specified to avoid repeated lookups.
     *
     * @return status (bit pattern) of the mc match
     */
    int getMCTruthStatus(const Particle* particle, const MCParticle* mcParticle = nullptr);

    /** Sets mctruth flag in MCTruthStatus extrainfo (also returns it).
     *
     * Users should use getMCTruthStatus(), which only calculates this information when necessary.
     *
     * */
    int setMCTruthStatus(Particle* particle, const MCParticle* mcParticle);

    /**
     * Fills vector with array (1-based) indices of all generator ancestors of given MCParticle.
     *
     * @param pointer to the MCParticle
     * @param reference to the vector of integers to hold the results
     */
    void fillGenMothers(const MCParticle* mcP, std::vector<int>& genMCPMothers);

    /**
     * Finds a mother of mcP that is in firstMothers, from [lastMother,  end]
     *
     * To actually find the common mother of all daughters, each time this function is called for a daughter particle, specify the return value from the last call for lastMother.
     *
     * @return index of the first common mother in firstMothers (!), or -1 if not found.
     */
    int findCommonMother(const MCParticle* mcP, const std::vector<int>& firstMothers, int lastMother);

    /**
     * Appends final state particle (FSP) to the vector of Particles.
     *
     * @param pointer to the particle
     * @param vector of FSPs that are used to reconstruct specified particle
     */
    void appendFSP(const Particle* p,     std::vector<const Particle*>&   children);

    /**
     * Appends final state particle (FSP) to the vector of MCParticles.
     *
     * @param gen pointer to the MCParticle
     * @param children vector of FSPs that can be found in the generated decay chain of specified MCParticle
     */
    void appendFSP(const MCParticle* gen, std::vector<const MCParticle*>& children);

    /**
     * Returns true if given MCParticle is FSP or not.
     *
     * @return true if MCParticle is FSP and false otherwise
     */
    bool isFSP(const MCParticle* p);

    /**
     * Finds final state particles given in vector of generated particles that
     * are not given in vector of reconstructed final state particles,
     * returns flags describing what kind of FSPs are missing.
     *
     * @param reconstructedFSPs vector of reconstructed final state particles
     * @param generatedFSPs vector of generated final state particles
     * @returns ORed combination of MCMatchStatus flags for missing particles.
     */
    int getMissingParticleFlags(const std::vector<const Particle*>& reconstructedFSPs, const std::vector<const MCParticle*>& generatedFSPs);
  }
}
