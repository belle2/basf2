/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Utility.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>

#include <utility>

#include <boost/functional/hash/extensions.hpp>

namespace std {
  /** Hash function used by unordered_set. */
  template<> struct hash<std::set<int>> {
    /** Actual work done by boost. */
    std::size_t operator()(const std::set<int>& v) const
    {
      return boost::hash_value(v);
    }
  };
}

namespace Belle2 {

  /**
  * ParticleIndexGenerator is a generator for all the combinations of the particle indices stored in the particle lists.
  * It takes the total number of particles in each of the input lists e.g. 2,3 and returns all the possible combinations: 00, 01, 02, 10, 11, 12
  */
  class ParticleIndexGenerator {

  public:
    /**
     * Default constructor
     */
    ParticleIndexGenerator() : m_numberOfLists(0), m_iCombination(0), m_nCombinations(0) { }

    /**
     * Initialises the generator to produce combinations with the given sizes of each particle list
     * @param _sizes the sizes of the particle lists to combine
     */
    void init(const std::vector<unsigned int>& _sizes);

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /**
     * Returns theindices of the current loaded combination
     */
    const std::vector<unsigned int>& getCurrentIndices() const;


  private:

    unsigned int m_numberOfLists;  /**< Number of lists which are combined */
    unsigned int m_iCombination;         /**< The current position of the combination */
    unsigned int m_nCombinations;        /**< The total amount of combinations */
    std::vector<unsigned int> indices; /**< The indices of the current loaded combination */
    std::vector<unsigned int> sizes;   /**< The sizes of the particle lists which are combined */

  };

  /**
   * ListIndexGenerator is a generator for all the combinations of the sublists (FlavorSpecificParticle = 0, SelfConjugatedParticle = 1)
   * of a set of particle lists. Therefore it returns combinations like (for 2 Particle Lists):
   *   FlavorSpecificParticle,FlavorSpecificParticle,
   *   FlavorSpecificParticle, SelfConjugatedParticle
   *   SelfConjugatedParticle, FlavorSpecificParticle
   */
  class ListIndexGenerator {
  public:
    /**
     * Default constructor
     */
    ListIndexGenerator() : m_numberOfLists(0), m_iCombination(0), m_nCombinations(0) { }

    /**
     * Initialises the generator to produce the given type of sublist
     * @param _numberOfLists Number of Particle Lists which shall be combined
     */
    void init(unsigned int _numberOfLists);

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /**
     * Returns the type of the sublist of the current loaded combination
     */
    const std::vector<ParticleList::EParticleType>& getCurrentIndices() const;

  private:

    unsigned int m_numberOfLists; /**< Number of lists which are combined */
    unsigned int m_iCombination; /**< The current position of the combination */
    unsigned int m_nCombinations; /**< The total amount of combinations */
    std::vector<ParticleList::EParticleType> m_types; /**< The current types of sublist of the ParticleLists for this combination */

  };

  /**
   * ParticleGenerator is a generator for all the particles combined from the given ParticleLists.
   */
  class ParticleGenerator {
  public:

  public:
    /**
     * Initialises the generator to produce the given type of sublist
     * @param decayString
     * @param cutParameter
     */
    explicit ParticleGenerator(const std::string& decayString, const std::string& cutParameter = "");

    /**
     * Initialises the generator to produce the given type of sublist
     * @param decaydescriptor
     * @param cutParameter
     */
    explicit ParticleGenerator(const DecayDescriptor& decaydescriptor, const std::string& cutParameter = "");

    /**
     * Initialises the generator to produce the given type of sublist
     */
    void init();

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext(bool loadAntiParticle = true);

    /**
     * Returns the particle
     */
    Particle getCurrentParticle() const;

    /**
     * True if input lists collide (can contain copies of particles in the input lists).
     * When this is true the combiner uses specially determined unique particle's IDs instead
     * of its StoreArray indices in order to check the uniqueness of each combination.
     *
     * The function is needed only to (unit) test correct running of the combiner.
     */
    bool inputListsCollide() const {return m_inputListsCollide; };

    /**
     * True if the pair of input lists collide.
     *
     * Needed only for tests.
     */
    bool inputListsCollide(const std::pair<unsigned, unsigned>& pair) const;

    /**
     * Returns the unique ID assigned to Particle with given index from the IndicesToUniqueID map.
     * If the Particle's index is not found in the map -1 is returned instead.
     * If the map was never filled (inputListsCollide is false) 0 is returned.
     *
     * Needed only for tests.
     */
    int getUniqueID(int index) const;

  private:
    /**
     * Create current particle object
     */
    Particle createCurrentParticle() const;

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNextParticle(bool useAntiParticle);

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNextSelfConjugatedParticle();

    /**
     * Check that all FS particles of a combination differ. The comparison is made
     * at the MDST objects level. If for example a kaon and a pion Particle's are
     * created from the same MSDT Track object, then these two Particles have the
     * same source and the function will return false.
     *
     * @return true if all FS particles of a combination differ
     */
    bool currentCombinationHasDifferentSources();

    /**
     * Check that the combination is unique. Especially in the case of reconstructing
     * self conjugated decays we get combinations like M -> A B and M -> B A. These two
     * particles are the same and hence only one combination of the two should be kept.
     * This function takes care of this. It keeps track of all combinations that were
     * already accepted (ordered set of unique IDs of daughter particles) and if the
     * current combination is already found in the set it is discarded. The unique ID
     * of daughter particles are either their StoreArray indices (if input lists do not
     * collide) or specially set unique IDs during initialization phase (if input lists
     * collide).
     *
     * @return true if indices not found in the stack; if true indices pushed to stack
     */
    bool currentCombinationIsUnique();

    /**
     * Check that: if the current combination has at least two particles from an ECL source,
     * then they are from different connected regions or from the same connected region but have
     * the same hypothesis.
     *
     * @return true if indices not found in the stack; if true indices pushed to stack
     */
    bool currentCombinationIsECLCRUnique();

    /**
     * In the case input daughter particle lists collide (two or more lists contain copies of Particles)
     * the Particle's Store Array index can not be longer used as its unique identifier, which is needed
     * to check for uniqueness of accpeted combinations. Instead unique identifier is created for all particles
     * in the input particle lists and use those when checking for uniqueness of current combination.
     */
    void initIndicesToUniqueIDMap();

    /**
     * Assigns unique IDs to all particles in list A, which do not have the unique ID already assigned. The same unique ID
     * is assigned to copies of particles from list A found in the list B. This function has to be executed first.
     *
     */
    void fillIndicesToUniqueIDMap(const std::vector<int>& listA, const std::vector<int>& listB, int& uniqueID);

    /**
     * Assigns unique IDs to all particles in list A, which do not have the unique ID already assigned.
     */
    void fillIndicesToUniqueIDMap(const std::vector<int>& listA, int& uniqueID);

  private:

    int m_pdgCode; /**< PDG Code of the particle which is combined */
    bool m_isSelfConjugated; /**< True if the combined particle is self-conjugated */
    unsigned int m_iParticleType; /**< The type of particle which is currently generated */
    int m_properties; /**< Particle property. Flags are defined in Particle::PropertyFlags */
    std::vector<int> m_daughterProperties; /**< Daughter's particle properties. */

    unsigned int m_numberOfLists; /**< Number of lists which are combined */
    std::vector<StoreObjPtr<ParticleList>> m_plists; /**< particle lists */

    ListIndexGenerator
    m_listIndexGenerator;   /**< listIndexGenerator makes the combinations of the types of sublists of the ParticleLists */
    ParticleIndexGenerator
    m_particleIndexGenerator; /**< particleIndexGenerator makes the combinations of indices stored in the sublists of the ParticleLists */

    const StoreArray<Particle> m_particleArray; /**< Global list of particles. */
    std::vector<Particle*> m_particles; /**< Pointers to the particle objects of the current combination */
    std::vector<int> m_indices;         /**< Indices stored in the ParticleLists of the current combination */
    std::unordered_set<std::set<int>> m_usedCombinations; /**< already used combinations (as sets of indices or unique IDs). */

    bool m_inputListsCollide; /**< True if the daughter lists can contain copies of Particles */
    std::vector<std::pair<unsigned, unsigned>> m_collidingLists; /**< pairs of lists that can contain copies. */
    std::unordered_map<int, int>
    m_indicesToUniqueIDs; /**< map of store array indices of input Particles to their unique IDs. Necessary if input lists collide. */

    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    Particle m_current_particle; /**< The current Particle object generated by this combiner */
  };


}
