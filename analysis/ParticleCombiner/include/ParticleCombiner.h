/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vector>
#include <set>
#include <unordered_set>

#include <boost/functional/hash/extensions.hpp>

namespace std {
  /** Hash function used by unordered_set. */
  template<> struct hash<std::set<int>> {
    /** Actual work done by boost. */
    std::size_t operator()(const std::set<int>& v) const {
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
     * Initialises the generator to produce combinations with the given sizes of each particle list
     * @param sizes the sizes of the particle lists to combine
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

    unsigned int numberOfLists;  /**< Number of lists which are combined */
    unsigned int iCombination;         /**< The current position of the combination */
    unsigned int nCombinations;        /**< The total amount of combinations */
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

  public:
    /**
     * Initialises the generator to produce the given type of sublist
     * @param numberOfLists Number of Particle Lists which shall be combined
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

    unsigned int numberOfLists; /**< Number of lists which are combined */
    unsigned int iCombination; /**< The current position of the combination */
    unsigned int nCombinations; /**< The total amount of combinations */
    std::vector<ParticleList::EParticleType> types; /**< The current types of sublist of the ParticleLists for this combination */

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
     */
    ParticleGenerator(std::string decayString);

    /**
     * Initialises the generator to produce the given type of sublist
     */
    void init();

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /**
     * Returns the particle
     */
    Particle getCurrentParticle() const;

  private:
    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNextParticle(bool useAntiParticle);

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNextSelfConjugatedParticle();

    /**
     * Check that all FS particles of a combination differ
     * @return true if all FS particles of a combination differ
     */
    bool currentCombinationHasDifferentSources();

    /**
     * Check that the combination is unique
     * @return true if indices not found in the stack; if true indices pushed to stack
     */
    bool currentCombinationIsUnique();

  private:

    int m_pdgCode; /**< PDG Code of the particle which is combined */
    bool m_isSelfConjugated; /**< True if the combined particle is self-conjugated */
    unsigned int iParticleType; /**< The type of particle which is currently generated */

    unsigned int numberOfLists; /**< Number of lists which are combined */
    std::vector<StoreObjPtr<ParticleList>> m_plists; /**< particle lists */

    ListIndexGenerator listIndexGenerator;   /**< listIndexGenerator makes the combinations of the types of sublists of the ParticleLists */
    ParticleIndexGenerator particleIndexGenerator; /**< particleIndexGenerator makes the combinations of indices stored in the sublists of the ParticleLists */

    const StoreArray<Particle> m_particleArray; /**< Global list of particles. */
    std::vector<Particle*> m_particles; /**< Pointers to the particle objects of the current combination */
    std::vector<int> m_indices;         /**< Indices stored in the ParticleLists of the current combination */
    std::unordered_set<std::set<int>> m_usedCombinations; /**< already used combinations (as sets of indices). */

  };


}
