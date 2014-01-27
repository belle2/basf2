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
#include <vector>

namespace Belle2 {

  class Particle;

  /** \addtogroup dataobjects
   * @{
   */

  /**
   * Class to hold a list of particles and anti-particles. The list is implemented
   * as two std::vector<int> holding the indices of particles in StoreArray<Particle>.
   * Particles in the list can only be of the same kind (according to PDG code and
   * flavor type) and from the same Particle store array.
   */

  class ParticleList : public TObject {
  public:

    /**
     * Default constructor
     */
    ParticleList():
      m_pdg(0),
      m_flavorType(0),
      m_particleStore("Particles")
    {}

    /**
     * Sets PDG code
     * @param pdg PDG code
     */
    void setPDG(int pdg) {m_pdg = pdg; setFlavorType();}

    /**
     * Sets Particle store array name to which particle list refers
     * @param name name of the Particle store array
     */
    void setParticleCollectionName(std::string name) {m_particleStore = name;}

    /**
     * Swaps flavor type
     */
    void swapFlavorType() {m_flavorType = (m_flavorType == 0) ? 1 : 0;}

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
     * @param K 0=particle, 1=anti-particle
     */
    void addParticle(unsigned iparticle, unsigned K) {
      K = K < m_flavorType ? K : m_flavorType;
      m_list[K].push_back(iparticle);
      m_good[K].push_back(true);
    }

    /**
    * Mark element i in vector m_list[K] for removal from the list.
    * Marked elements are removed afterwards by calling removeMarked().
    * @param i list element
    * @param K vector index 0 or 1
    */
    void markToRemove(unsigned i, unsigned K);

    /**
    * Mark i-th particle for removal from the list.
    * Marked elements are removed afterwards by calling removeMarked().
    * @param i list index (i<getListSize())
    */
    void markToRemove(unsigned i);

    /**
     * Remove marked elements from list
     */
    void removeMarked();

    /**
     * Returns Particle store array name to which particle list refers
     * @return name of the Particle store array
     */
    std::string getParticleCollectionName() {return m_particleStore;}

    /**
     * Returns PDG code
     * @return PDG code
     */
    int getPDG() const {return m_pdg;}

    /**
     * Returns PDG code of anti-particle
     * @return PDG code of anti-particle
     */
    int getPDGbar() const {return m_flavorType == 0 ? m_pdg : -m_pdg;}

    /**
     * Returns PDG code of particle or anti-particle according to the value of K
     * @param K selector (0=particle, 1=anti-particle)
     * @return PDG code
     */
    int getPDG(unsigned K) const {return K == 0 ? getPDG() : getPDGbar();}

    /**
     * Returns flavor type
     * @return flavor type
     */
    unsigned getFlavorType() const {return m_flavorType;}

    /**
     * Returns list of StoreArray<Particle> indices.
     * @param K selector (0=particle, 1=anti-particle)
     * @return const reference to vector of indices
     */
    const std::vector<int>& getList(unsigned K) const {
      K = K < m_flavorType ? K : m_flavorType;
      return m_list[K];
    }

    /**
     * Returns list size
     * @return list size
     */
    unsigned getListSize() const {
      return m_list[0].size() + m_list[1].size();
    }

    /**
     * Returns i-th particle from the list
     * @param i list index (i < getListSize())
     * @return pointer to Particle or NULL
     */
    Particle* getParticle(unsigned i) const;

    /**
     * Returns number of particles in the list
     * @return number of particles
     */
    unsigned getNumofParticles() const {
      return m_list[0].size();
    }

    /**
     * Returns number of anti-particles in the list
     * @return number of anti-particles
     */
    unsigned getNumofAntiParticles() const {
      return m_list[1].size();
    }

    /**
     * Prints the list
     */
    void print() const;

  private:

    int m_pdg;                         /**< PDG code */
    unsigned m_flavorType;             /**< 0 unflavored, 1 flavored */
    std::vector<int> m_list[2];        /**< list of 0-based indices of Particles */
    std::vector<bool> m_good[2];       /**< flag for 'good' element */
    std::string m_particleStore;       /**< name of Particle store array */

    /**
     * sets m_flavorType using m_pdg
     */
    void setFlavorType();

    ClassDef(ParticleList, 2); /**< ClassDef */

  };

  /** @}*/

} // end namespace Belle2

#endif
