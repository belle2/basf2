/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/ParticleList.h>

#include <framework/datastore/StoreObjPtr.h>

#include <vector>
#include <unordered_set>
#include <algorithm>

#include <boost/functional/hash.hpp>

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
  class Particle;

  /**
   * ListCombiner is a generator for all the combinations of the sublists (Particle,AntiParticle, SelfConjugatedParticle)
   * of a set of particle lists. Therefore it returns combinations like (for 2 Particle Lists):
   * For currentType == Particle:
   *   Particle,Particle,
   *   Particle, SelfConjugatedParticle
   *   SelfConjugatedParticle, Particle
   * For currentType == AntiParticle
   *   AntiParticle, AntiParticle
   *   AntiParticle, SelfConjugatedParticle
   *   SelfConjugatedParticle, AntiParticle
   * Fpr currentType == SelfConjugatedParticle
   *   SelfConjugatedParticle, SelfConjugatedParticle
   */
  class ListCombiner {
  public:
    /**
     * Constructor
     * @param numberOfLists Number of Particle Lists which shall be combined
     */
    ListCombiner(unsigned int numberOfLists);

    /**
     * Initialises the generator to produce the given type of sublist
     * @param _currentType the type of sublist which is produced by the combination of ParticleLists
     */
    void init(ParticleList::EParticleType _currentType);

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /**
     * Returns the type of the sublist of the current loaded combination
     */
    const std::vector<ParticleList::EParticleType>& getCurrentTypes() const;

  private:
    const unsigned int numberOfLists; /**< Number of lists which are combined */
    unsigned int iCombination; /**< The current position of the combination */
    unsigned int nCombinations; /**< The total amount of combinations */
    ParticleList::EParticleType currentType; /**< The current type of sublist which is produced by the combination */
    std::vector<ParticleList::EParticleType> types; /**< The current types of sublist of the ParticleLists for this combination */

  };

  /**
   * IndexCombiner is a generator for all the combinations of the particle indices stored in the particle lists
   */
  class IndexCombiner {

  public:
    /**
     * Constructor
     * @param numberOfLists Number of Particle Lists which shall be combined
     */
    IndexCombiner(unsigned int numberOfLists);

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
    const unsigned int numberOfLists; /**< Number of lists which are combined */
    unsigned int iCombination; /**< The current position of the combination */
    unsigned int nCombinations; /**< The total amount of combinations */
    std::vector<unsigned int> indices; /**< The indices of the current loaded combination */
    std::vector<unsigned int> sizes; /**< The sizes of the particle lists which are combined */

  };

  /**
   * ParticleCombiner combines ParticleLists to a new ParticleList using the ListCombiner and IndexCombiner.
   *
   * \note This class retains state, so create a new ParticleCombiner object for each event.
   */
  class ParticleCombiner {

  public:
    /**
     * Constructor
     * @param inputListNames the names of the ParticleLists. These ParticleLists have to be in the DataStore
     * @param isCombinedParticleSelfConjugated If the combined particle is self conjugated the produced ParticleList by this combiner is also SelfConjugated (so only the sublist SelfConjugatedParticle is filled)
     */
    ParticleCombiner(const std::vector<std::string>& inputListNames, bool isCombinedParticleSelfConjugated);

    /*
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /*
     * Returns the current loaded combination of particles as pointers to the particle objects
     */
    const std::vector<Particle*>& getCurrentParticles() const { return m_particles; }

    /**
     * Returns the current loaded combination of particles as indices which are found in the given ParticleLists
     */
    const std::vector<int>& getCurrentIndices() const { return m_indices; }

    /**
     * Return the ParticleType of the current combination. If the Particle which is combined
     * is selfConjugated itself, all combinations have to be ParticleList::c_SelfConjugatedParticle.
     */
    ParticleList::EParticleType getCurrentType() const;

  private:
    /**
     * Returns the type of sublist which is currently produced, only difference to getCurrentType()
     * is that this function doesn't consider the isCombinedParticleSelfConjugated flag given in the constructor
     */
    ParticleList::EParticleType getCurrentCombinationType() const;

    /**
     * Checks if given daughter particles are self conjugated
     * @param plists refernce to vector of input particle lists StoreObjPtr's
     */
    bool isDecaySelfConjugated(std::vector<StoreObjPtr<ParticleList> >& plists);

    /**
     * Loads the next combination provided by the IndexCombiner
     */

    bool loadNextCombination();

    /**
     * Loads the next type of output sublist
     */
    bool loadNextType();

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

    ListCombiner listCombiner; /**< ListCombiner makes the combinations of the types of sublists of the ParticleLists */
    IndexCombiner indexCombiner; /**< IndexCombiner makes the combinations of indices stored in the sublists of the ParticleLists */

    std::vector<StoreObjPtr<ParticleList> > plists; /**< Store array pointers to the ParticleLists which should be combined */
    std::vector<Particle*> m_particles; /**< Pointers to the particle objects of the current combination */
    std::vector<int> m_indices; /**< Indices stored in the ParticleLists of the current combination */
    std::unordered_set<std::set<int>> m_usedCombinations; /**< already used combinations (as sets of indices). */

    unsigned int numberOfLists; /**< Number of lists which are combined */

    int nTypes; /**< Total amount of types of sublists. So... 3! */
    int iType; /**< Current type of sublist which is produced */

    bool m_isCombinedParticleSelfConjugated; /**< True if the combined particle is self-conjugated */


  };

}
