/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLELIST_H
#define PARTICLELIST_H

#include <TObject.h>
#include <TParticlePDG.h>
#include <vector>

namespace Belle2 {

  class Particle;

  /** \addtogroup dataobjects
   * @{
   */


  /**
   * Class to hold a list of particles, anti-particles and self-conjugated particles.
   * The list is implemented as three std::vector<int> holding the indices of particles in StoreArray<Particle>.
   * Particles in the list can only be of the same kind (according to PDG code) and from the same Particle store array.
   */

  class ParticleList : public TObject {
  public:

    enum EParticleType {
      c_Particle = 0,
      c_AntiParticle,
      c_SelfConjugatedParticle
    };

    /**
     * Default constructor
     */
    ParticleList():
      m_pdg(0),
      m_pdgbar(0),
      m_particleStore("Particles")
    {}

    /**
     * Sets PDG code
     * @param pdg PDG code
     */
    void setPDG(int pdg);

    /**
     * Sets Particle store array name to which particle list refers
     * @param name name of the Particle store array
     */
    void setParticleCollectionName(std::string name) {m_particleStore = name;}

    /**
     * Adds a new particle to the list (safe method)
     * @param particle pointer to particle in the StoreArray<Particle>
     */
    void addParticle(const Particle* particle);

    /**
     * Adds a new particle to the list (almost safe method)
     * @param iparticle index of the particle in the StoreArray<Particle>
     * @param pdg particle PDG code
     * @param flavorType particle flavor type
     */
    void addParticle(unsigned iparticle, int pdg, unsigned flavorType);

    /**
     * Adds a new particle to the list (unsafe method - please, use the safe one).
     * Particle must be of the kind the list is constructed for.
     * To be used in ParticleCombiner only.
     * @param iparticle index of the particle in the StoreArray<Particle>
     * @param K ParticleType - Particle, AntiParticle or SelfConjugatedParticle
     */
    void addParticle(unsigned iparticle, EParticleType K) {
      m_list[K].push_back(iparticle);
    }

    /**
     * Remove given elements from list
     */
    void removeParticles(const std::vector<unsigned int>& toRemove);

    /**
     * Returns Particle store array name to which particle list refers
     * @return name of the Particle store array
     */
    std::string getParticleCollectionName() {return m_particleStore;}

    /**
     * Returns PDG code
     * @return PDG code
     */
    int getPDG() const {return m_pdg; }

    /**
     * Returns PDG code of anti-particle
     * @return PDG code of anti-particle
     */
    int getPDGbar() const { return m_pdgbar; }

    /**
     * Returns list of StoreArray<Particle> indices.
     * @param K ParticleType - Particle, AntiParticle or SelfConjugatedParticle
     * @return const reference to vector of indices
     */
    const std::vector<int>& getList(EParticleType K) const {
      return m_list[K];
    }

    /**
     * Returns list size
     * @return list size
     */
    unsigned getListSize() const {
      return m_list[0].size() + m_list[1].size() + m_list[2].size();
    }

    /**
     * Returns i-th particle from the list
     * @param i list index (i < getListSize())
     * @return pointer to Particle or NULL
     */
    Particle* getParticle(unsigned i) const;

    /**
     * Returns number of particles, antiparticles or self-conjugated particles in the list
     * @param K ParticleType - Particle, AntiParticle or SelfConjugatedParticle
     * @return number of particles, antiparticles or self-conjugated particles
     */
    unsigned getNumOf(EParticleType K) const {
      return m_list[K].size();
    }

    /**
     * Prints the list
     */
    void print() const;

  private:

    /**
     * Checks if the given pdg code particle has an anti particle
     * We can't use TDatabase here because of a bug in our patch of ROOT!
     */
    bool hasAntiParticle(int pdg);

  private:

    int m_pdg;               /**< PDG code of Particle */
    int m_pdgbar;               /**< PDG code of antiparticle */
    std::vector<int> m_list[3];        /**< list of 0-based indices of Particles */
    std::string m_particleStore;       /**< name of Particle store array */

    ClassDef(ParticleList, 2); /**< ClassDef */

  };

  /** @}*/

} // end namespace Belle2

#endif
