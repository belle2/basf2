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

  class ListCombiner {
  public:
    ListCombiner(unsigned int numberOfLists);
    void init(ParticleList::EParticleType _currentType);
    bool loadNext();
    const std::vector<ParticleList::EParticleType>& getCurrentTypes() const;

  private:
    const unsigned int numberOfLists;
    unsigned int iCombination;
    unsigned int nCombinations;
    ParticleList::EParticleType currentType;
    std::vector<ParticleList::EParticleType> types;

  };

  class IndexCombiner {

  public:
    IndexCombiner(unsigned int numberOfLists);

    void init(const std::vector<unsigned int>& _sizes);
    bool loadNext();
    const std::vector<unsigned int>& getCurrentIndices() const;


  private:
    const unsigned int numberOfLists;
    unsigned int iCombination;
    unsigned int nCombinations;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> sizes;

  };

  class ParticleCombiner {

  public:
    ParticleCombiner(const std::vector<std::string>& inputListNames, bool isCombinedParticleSelfConjugated);

    bool loadNext();

    const std::vector<Particle*>& getCurrentParticles() const { return m_particles; }
    const std::vector<int>& getCurrentIndices() const { return m_indices; }

    /**
     * Return the ParticleType of the current combination. If the Particle which is combined
     * is selfConjugated itself, all combinations have to be ParticleList::c_SelfConjugatedParticle,
     * however the ParticleCombiner doesn't have any information on the particle it combines,
     * so if you combine a self conjugated particle you have to fix the Type by yourself
     */
    ParticleList::EParticleType getCurrentType() const;

  private:
    ParticleList::EParticleType getCurrentCombinationType() const;

    /**
     * Checks if given daughter particles are self conjugated
     * @param plists refernce to vector of input particle lists StoreObjPtr's
     */
    bool isDecaySelfConjugated(std::vector<StoreObjPtr<ParticleList> >& plists);

    bool loadNextCombination();
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

    ListCombiner listCombiner;
    IndexCombiner indexCombiner;

    std::vector<StoreObjPtr<ParticleList> > plists;
    std::vector<Particle*> m_particles;
    std::vector<int> m_indices;
    std::unordered_set<std::set<int>> m_usedCombinations; /**< already used combinations (as sets of indices). */

    unsigned int numberOfLists;

    int nTypes;
    int iType;

    bool isCombinedParticleSelfConjugated;


  };

}
