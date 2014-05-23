// ******************************************************************
// MC Matching
// authors: A. Zupanc (anze.zupanc@ijs.si), C. Pulvermacher (christian.pulvermacher@kit.edu)
// ******************************************************************

#ifndef MCPARTICLEMATCHING_H
#define MCPARTICLEMATCHING_H

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
      c_MissFSR             = 1,  /**< bit 0: A Final State Radiation (FSR) photon is not reconstructed. */
      c_MisID               = 2,  /**< bit 1: One of the charged final state particles is mis-identified. */
      c_MissGamma           = 4,  /**< bit 2: A Photon (not FSR) is missing (not reconstructed). */
      c_MissMassiveParticle = 8,  /**< bit 3: A generated massive particle is missing (not reconstructed). */
      c_MissNeutrino        = 16, /**< bit 4: A Neutrino is missing (not reconstructed). */
      c_MissKlong           = 32, /**< bit 5: A Klong is missing (not reconstructed). */
      c_AddedWrongParticle  = 64,  /**< bit 6: A non-FSP Particle has wrong PDG code, meaning one of the daughters (or their daughters) belongs to another Particle. */
      c_DecayInFlight       = 128,  /**< bit 7: A Particle was reconstructed from the secondary decay product of the actual particle. This means that a wrong hypothesis was used to reconstruct it, which e.g. for tracks might mean a pion hypothesis was used for a secondary electron. */
      c_InternalError       = 256,  /**< bit 8: There was an error in MC matching. Not a valid match. */
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
     * Finds index of a MCParticle that represents the first common mother (ancestor) of all daughter particles.
     *
     * @param number of daughter particles
     * @param indices of all generated ancestors of the first daughter particle
     * @param indices of all generated ancestors of all other daughter particles
     *
     * @return index of the first common mother
     */
    int findCommonMother(unsigned nChildren, const std::vector<int>& firstMothers, const std::vector<int>& otherMothers);

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
     * Finds final stat particles given in vector of generated particles that
     * are not given in vector of reconstructed final state particles (in other
     * words: finds out missing what are the missing particles) and saves their
     * indices.
     *
     * @param reconstructed vector of reconstructed final state particles
     * @param generated vector of generated final state particles
     * @param missP vector of generated final state particles(holds results of this algorithm)
     */
    void findMissingGeneratedParticles(std::vector<const Particle*> reconstructed,
                                       std::vector<const MCParticle*> generated, std::vector<const MCParticle*>& missP);


    /**
     * Determines whether (true) or not (false) the reconstructed particle misses generated Final State Radiation (FSR) photon.
     *
     * @param missP vector of generated final state particles
     *
     * @return whether (true) or not (false) the reconstructed particle misses generated Final State Radiation (FSR) photon
     */
    bool missingFSRPhoton(std::vector<const MCParticle*> missP);

    /**
     * Determines whether (true) or not (false) the reconstructed particle misses generated radiative photon.
     *
     * @param missP vector of generated final state particles
     *
     * @return whether (true) or not (false) the reconstructed particle misses radiative photon
     */
    bool missingRadiativePhoton(std::vector<const MCParticle*> missP);

    /**
     * Determines whether (true) or not (false) the reconstructed particle misses generated neutrino.
     *
     * @param missP vector of generated final state particles
     *
     * @return whether (true) or not (false) the reconstructed particle misses neutrino
     */
    bool missingNeutrino(std::vector<const MCParticle*> missP);

    /**
     * Determines whether (true) or not (false) the reconstructed particle misses massive particle.
     *
     * @param missP vector of generated final state particles
     *
     * @return whether (true) or not (false) the reconstructed particle misses massive particle
     */
    bool missingMassiveParticle(std::vector<const MCParticle*> missP);

    /**
     * Determines whether (true) or not (false) the reconstructed particle misses Klong.
     *
     * @param missP vector of generated final state particles
     *
     * @return whether (true) or not (false) the reconstructed particle misses Klong
     */
    bool missingKlong(std::vector<const MCParticle*> missP);
  }
}

#endif // MCPARTICLEMATCHING
