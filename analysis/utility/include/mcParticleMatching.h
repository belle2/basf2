// ******************************************************************
// MC Matching
// author: A. Zupanc (anze.zupanc@ijs.si)
// ******************************************************************

#ifndef MCPARTICLEMATCHING_H
#define MCPARTICLEMATCHING_H

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>

#include <vector>

using namespace Belle2;
using namespace std;


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
bool setMCTruth(const Particle* particle);

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
int getMCTruthStatus(const Particle* particle, const MCParticle* mcParticle);

/**
 * Returns the flag (quality indicator) of the match. Possible values and meaning are:
 * -11 : gen_hepevt link of one of children not found in list of daughters of matched gen_hepevt (something wrong)
 * -10 : one of children has not link to gen_hepevt (something wrong)
 *  -9 : as -10 but in addition size of reconstructed < size of generated
 *  -5 : more reconstructed than generated particle in final state (something wrong)
 *  -2 : gen_hepvt particle doesn't have any daughters (something wrong in maching)
 *  -1 : random combination (common mother is virtual gamma (ccbar, uds mc), or Upsilon(4S), Upsilon(5S))
 *   0 : no link to gen_hepevt particle
 *   1 : particle is correctly reconstructed; including correct particle id of final state particles (SIGNAL)
 *   2 : one or more FSP are misidentified, but have common mother
 *   3 : FSP have common mother, but at least one massive particle is missing
 *   4 : FSP have common mother, but at least one massless particle is missing (radiative photon)
 *   5 : final state includes pi0 without perfect match to gen_hepevt
 *   6 : ID = 2 and 5 are true
 *  10 : particle is correctly reconstructed; including correct particle id of final state particles, but FSR photon is missing (SIGNAL)
 *  11 : in addition to FSR photon missing one more radiative photon is missing
 *  20 : missing neutrino
 *  21 : missing neutrino and radiative photon
 *  23 : missing neutrino and massive particle
 *  24 : missing neutrino and another massles particle (FSR photon)
 *
 * @param pointer to the particle
 * @param pointer to the matched MCParticle
 *
 * @return flag of the mc match
 */
int  getMCTruthFlag(const Particle* particle, const MCParticle* mcParticle);

/**
 * Fills vector with array (1-based) indices of all generator ancestors of given MCParticle.
 *
 * @param pointer to the MCParticle
 * @param reference to the vector of integers to hold the results
 */
void fillGenMothers(const MCParticle* mcP, vector<int>& genMCPMothers);

/**
 * Finds index of a MCParticle that represents the first common mother (ancestor) of all daughter particles.
 *
 * @param number of daughter particles
 * @param indices of all generated ancestors of the first daughter particle
 * @param indices of all generated ancestors of all other daughter particles
 *
 * @return index of the first common mother
 */
int findCommonMother(unsigned nChildren, vector<int> firstMothers, vector<int> otherMothers);

/**
 * Appends final state particle (FSP) to the vector of Particles.
 *
 * @param pointer to the particle
 * @param vector of FSPs that are used to reconstruct specified particle
 */
void appendFSP(const Particle* p,     vector<const Particle*>&   children);

/**
 * Appends final state particle (FSP) to the vector of MCParticles.
 *
 * @param pointer to the MCParticle
 * @param vector of FSPs that can be found in the generated decay chain of specified MCParticle
 */
void appendFSP(const MCParticle* gen, vector<const MCParticle*>& children);

/**
 * Returns true if given MCParticle is FSP or not.
 *
 * @return true if MCParticle is FSP and false otherwise
 */
int isFSP(const MCParticle* P);

/**
 * Determines the flag. See above.
 *
 * @return the mc match flag
 */
int compareFinalStates(vector<const Particle*> reconstructed, vector<const MCParticle*> generated);

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
void findMissingGeneratedParticles(vector<const Particle*> reconstructed, vector<const MCParticle*> generated, std::vector<int>& missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses generated Final State Radiation (FSR) photon.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses generated Final State Radiation (FSR) photon
 */
bool missingFSRPhoton(vector<const MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses generated radiative photon.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses radiative photon
 */
bool missingRadiativePhoton(vector<const MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses generated neutrino.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses neutrino
 */
bool missingNeutrino(vector<const MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses massive particle.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses massive particle
 */
bool missingMassiveParticle(vector<const MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) the reconstructed particle misses Klong.
 *
 * @param vector of generated final state particles
 * @param vector of indices of the missing generated final state particles
 *
 * @return whether (true) or not (false) the reconstructed particle misses Klong
 */
bool missingKlong(vector<const MCParticle*> generated, std::vector<int> missP);

/**
 * Determines whether (true) or not (false) any of the final state particles is mis-identified.
 *
 * @param vector of reconstructed final state particles
 * @param vector of generated final state particles
 *
 * @return whether (true) or not (false) any of the final state particles is mis-identified
 */
bool isMissidentified(vector<const Particle*> reconstructed, vector<const MCParticle*> generated);

#endif // MCPARTICLEMATCHING
