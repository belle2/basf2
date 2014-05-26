/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLELIST_H
#define PARTICLELIST_H

#include <TObject.h>
#include <vector>

#include <analysis/dataobjects/Particle.h>

#include <iostream>

namespace Belle2 {

  /**
   * Class to hold a list of particles, anti-particles and self-conjugated particles.
   * The list is implemented as three std::vector<int> holding the indices of particles in StoreArray<Particle>.
   * Particles in the list can only be of the same kind (according to PDG code) and from the same Particle store array.
   */

  class ParticleList : public TObject {
  public:

    /** Identifies the type of Particle (and in which of internal two lists it is stored). */
    enum EParticleType {
      c_FlavorSpecificParticle = 0,
      c_SelfConjugatedParticle
    };

    /**
     * Default constructor
     */
    ParticleList():
      m_pdg(0),
      m_pdgbar(0),
      m_particleStore("Particles"),
      m_thisListName(""),
      m_antiListName("") {
    }

    /**
     * Sets the PDG code and name of this ParticleList.
     *
     * At this point it is assumed that this is self-conjugated list (PDG code = anti-PDG code).
     * To bind the particle and anti-particle ParticleLists use bindAntiParticleList(ParticleList &antiList) method.
     *
     * @param pdg PDG code of particles to be hold in this list
     * @param name of the particle list
     */
    void initialize(int pdg, std::string name);

    /**
     * Binds particle and anti-particle ParticleLists.
     *
     * @param antiList - anti-particle ParticleList of this ParticleList
     */
    void bindAntiParticleList(ParticleList& antiList, bool includingAntiList = true);

    /**
     * Sets Particle store array name to which particle list refers
     * @param name name of the Particle store array
     */
    void setParticleCollectionName(std::string name, bool forAntiParticle = true);

    /**
     * Adds a new particle to the list (safe method)
     *
     * The method determines based on the particle's properties (PDGcode, flavorType)
     * to which particular list this particle belongs (flavor-specific or
     * self-conjugated particle or anti-particle list)
     *
     * @param particle pointer to particle in the StoreArray<Particle>
     */
    void addParticle(const Particle* particle);

    /**
     * Adds a new particle to the list (almost safe method)
     *
     * @param iparticle index of the particle in the StoreArray<Particle>
     * @param pdg particle PDG code
     * @param flavorType particle flavor type
     */
    void addParticle(unsigned iparticle, int pdg, Particle::EFlavorType flavorType, bool includingAntiList = true);

    /**
     * Remove given elements from list
     *
     * @param toRemove vector of Particle array indices to be removed
     * @param removeFromAntiList flag indicating whether to remove particle also from anti-particle list (default is true)
     */
    void removeParticles(const std::vector<unsigned int>& toRemove, bool removeFromAntiList = true);

    /**
     * Returns Particle store array name to which particle list refers
     */
    std::string getParticleCollectionName() const {return m_particleStore;}

    /**
     * Returns PDG code
     */
    int getPDGCode() const {return m_pdg; }

    /**
     * Returns PDG code of anti-particle
     */
    int getAntiParticlePDGCode() const { return m_pdgbar; }

    /**
     * Returns list of StoreArray<Particle> indices.
     *
     * @param K ParticleType - Particle, SelfConjugatedParticle
     * @param forAntiParticle - whether the Particle or SelfConjugatedParticle should be returned for this (false) or anti-particle list (true)
     * @return const reference to vector of indices
     */
    std::vector<int> getList(EParticleType K, bool forAntiParticle = false) const;

    /**
     * Returns the name the anti-particle ParticleList.
     */
    std::string getAntiParticleListName() const { return m_antiListName; }

    /**
     * Returns the name the anti-particle ParticleList.
     */
    std::string getParticleListName() const { return m_thisListName; }

    /**
     * Returns total number of particles in this list and anti list if requested
     */
    unsigned getListSize(bool includingAntiList = true) const;

    /**
     * Returns i-th particle from the this list and anti list if requested
     * @param i list index (i < getListSize())
     * @return pointer to Particle or NULL
     */
    Particle* getParticle(unsigned i, bool includingAntiList = true) const;

    /**
     * Returns number of particles or self-conjugated particles in the list or anti-particle list
     *
     * @param K ParticleType - Particle or SelfConjugatedParticle
     * @param forAntiParticle - whether the Particle or SelfConjugatedParticle should be returned for this (false) or anti-particle list (true)
     * @return number of particles or self-conjugated particles in the list or anti-particle list
     */
    unsigned getNParticlesOfType(EParticleType K, bool forAntiParticle = false) const {
      const std::vector<int> list = getList(K, forAntiParticle);
      return list.size();
    }

    /**
     * Prints the list
     */
    void print() const;

  private:

    /**
     * Adds particle to the anti-particle List.
     */
    void addParticleToAntiList(unsigned iparticle, int pdg, Particle::EFlavorType type);

  private:

    int m_pdg;                   /**< PDG code of Particle */
    int m_pdgbar;                /**< PDG code of antiparticle */
    std::vector<int> m_fsList;   /**< list of 0-based indices of flavor-specific Particles (particles that have an anti-particle) */
    std::vector<int> m_scList;   /**< list of 0-based indices of self-conjugated Particles (particles that do not have an anti-particle) */

    std::string m_particleStore; /**< name of Particle store array */

    std::string m_thisListName;  /**< name of this ParticleList */
    std::string m_antiListName;  /**< name of ParticleList for anti-particles */

    ClassDef(ParticleList, 3); /**< Class to hold a list of particles, anti-particles and self-conjugated particles. */

  };

} // end namespace Belle2

#endif
