// ******************************************************************
// MC Matching
// author: A. Zupanc (anze.zupanc@ijs.si)
// ******************************************************************

#ifndef MCPARTICLEMATCHING_H
#define MCPARTICLEMATCHING_H

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>

#include <vector>

/**
 * This is the main function of MC matching algorithm. When executed the algorithm
 * searches for first common generated mother (MCParticle) of this particle's daughters.
 * If such MCParticle is found the Particle <-> MCParticle relation is set between them.
 * If the relation between daughter particles and matched generated MC particle does
 * not exist the algorithm sets it also for them.
 *
 * @param pointer to the Particle to be mc-matched
 *
 * @return returns true if relation is set and false otherwise
 */
bool setMCTruth(const Belle2::Particle* particle);

/**
 * Returns the status (quality indicator) of the match. The status is given as a bit pattern,
 * where the individual bits indicate the following (see MCMatchStatus enum):
 *
 * - bit 0 [c_MissFSR]             : Final State Radiation (FSR) photon is missing
 * - bit 1 [c_MisID]               : One of the charged final state particles is mis-identified
 * - bit 2 [c_MissGamma]           : Photon (radiative photon and not FSR photon) is missing
 * - bit 3 [c_MissMassiveParticle] : Massive particle is missing
 * - bit 4 [c_MissNeutrino]        : Neutrino is missing
 * - bit 5 [c_MissKlong]           : Klong is missing
 *
 * Status equal to 0 indicates the perfect MC match (evrything OK).
 *
 * Status smaller than 0 indicates an error in matching.
 *
 * @param pointer to the particle
 * @param pointer to the matched MCParticle
 *
 * @return status (bit pattern) of the mc match
 */
int getMCTruthStatus(const Belle2::Particle* particle, const Belle2::MCParticle* mcParticle);

/**
 * Fills vector with array (1-based) indices of all generator ancestors of given MCParticle.
 *
 * @param pointer to the MCParticle
 * @param reference to the vector of integers to hold the results
 */
void fillGenMothers(const Belle2::MCParticle* mcP, std::vector<int>& genMCPMothers);

/**
 * Finds index of a MCParticle that represents the first common mother (ancestor) of all daughter particles.
 *
 * @param number of daughter particles
 * @param indices of all generated ancestors of the first daughter particle
 * @param indices of all generated ancestors of all other daughter particles
 *
 * @return index of the first common mother
 */
int findCommonMother(unsigned nChildren, std::vector<int> firstMothers, std::vector<int> otherMothers);

/**
 * Appends final state particle (FSP) to the vector of Particles.
 *
 * @param pointer to the particle
 * @param vector of FSPs that are used to reconstruct specified particle
 */
void appendFSP(const Belle2::Particle* p,     std::vector<const Belle2::Particle*>&   children);

/**
 * Appends final state particle (FSP) to the vector of MCParticles.
 *
 * @param pointer to the MCParticle
 * @param vector of FSPs that can be found in the generated decay chain of specified MCParticle
 */
void appendFSP(const Belle2::MCParticle* gen, std::vector<const Belle2::MCParticle*>& children);

/**
 * Returns true if given MCParticle is FSP or not.
 *
 * @return true if MCParticle is FSP and false otherwise
 */
int isFSP(const Belle2::MCParticle* P);

/**
 * Finds final stat particles given in vector of generated particles that
 * are not given in vector of reconstructed final state particles (in other
 * words: finds out missing what are the missing particles) and saves their
 * indices.
 *
 * @param vector of reconstructed final state particles
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles (holds results of this algorithm)
 */
void findMissingGeneratedParticles(std::vector<const Belle2::Particle*> reconstructed,
                                   std::vector<const Belle2::MCParticle*> generated, std::vector<int>& missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses generated Final State Radiation (FSR) photon.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses generated Final State Radiation (FSR) photon
 */
bool missingFSRPhoton(std::vector<const Belle2::MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses generated radiative photon.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses radiative photon
 */
bool missingRadiativePhoton(std::vector<const Belle2::MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses generated neutrino.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses neutrino
 */
bool missingNeutrino(std::vector<const Belle2::MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses massive particle.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses massive particle
 */
bool missingMassiveParticle(std::vector<const Belle2::MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses Klong.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses Klong
 */
bool missingKlong(std::vector<const Belle2::MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) any of the final state particles is mis-identified.
 *
 * @param vector of reconstructed final state particles
 * @param vector of generated final state particles
 *
 * @return whether (true) or not (false) any of the final state particles is mis-identified
 */
bool isMisidentified(std::vector<const Belle2::Particle*> reconstructed, std::vector<const Belle2::MCParticle*> generated);

#endif // MCPARTICLEMATCHING
