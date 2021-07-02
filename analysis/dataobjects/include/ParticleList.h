/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <framework/utilities/ArrayIterator.h>

#include <TObject.h>
#include <vector>


namespace Belle2 {
  template <class T> class StoreObjPtr;

  /**
   * ParticleList is a container class that stores a collection of Particle objects. The particles are internally
   * stored in two std::vector<int> holding the indices of particles in StoreArray<Particle>:
   * o) flavor specific particles (particles that have an anti-particle, e.g. pi-, K-, D+, D0)
   * o) self-conjugated particles (particles that do not have an anti-particle, e.g. pi0, phi, or particles
   *    that have an anti-particle but are reconstructed in self-conjugated decay mode, e.g. K*0 -> K0s pi0, or D0 -> K- K+, ...)
   * ParticleList can store only particles with same PDG code (which however can be reconstructed in different decay modes).
   *
   * Each ParticleList can and should be bound with its anti-ParticleList at the time of creation of the lists.
   * This bond for example enables automatic reconstruction of charged conjugated decays in ParticleCombiner module for example.
   *
   * The unique identifier of the ParticleList is its name. According to the naming convention the ParticleList's name
   * has to be of the form:
   *
   * listName = particle_name:label,
   *
   * where particle_name is the name of the particle as given in the evt.pdl and the label can be any string
   * indicating the selection criteria or decay mode (or anything else) used to reconstruct the particles. Examples are:
   * o) pi+:loose - pi+ candidates passing loose PID requirements
   * o) D0:kpi    - D0 candidates reconstructed in D0->Kpi decays
   *
   * <h1>Creating new ParticleList</h1>
   *
   * Particles and their anti-particles are stored in their own ParticleLists. Creation and connection of particle and
   * anti-particle lists is performed in the following way (example for K- and K+ lists):

     \code
     \\ create ParticleList for particles (K+)
     StoreObjPtr<ParticleList> pList("K+:all");
     pList.create();
     pList->initialize( 321, pList.getName());

     \\ create ParticleList for anti-particles (K-)
     StoreObjPtr<ParticleList> antipList("K-:all");
     antipList.create();
     antipList->initialize(-321, antipList.getName());

     \\ bind the two lists together
     pList->bindAntiParticleList(*antipList);
     \endcode
   *
   * <h1>Adding Particle to ParticleList</h1>
   *
   * The following rules apply:
   * o) ParticleList can contain only particles with same absolute value of the PDG code (e.g. only charged kaons, neutral D mesons, ...)
   * o) All particles have to be stored in the same StoreArray<Particle>
   *
   * Example:
   *
     \code
     pList->addParticle(particle);
     \endcode

   * where particle is a pointer to a Particle object. Note, if one adds anti-particle to ParticleLists for particles the
   * particle will be actually added to the ParticleList for anti-particles. The following two lines of code give same result
   *
     \code
     // adding anti-particle Particle object to ParticleList for particles
     pList->addParticle(antiParticle);

     // is the same as if the anti-particle Particle object is added to the ParticleList for anti-particles
     antiPlist->addParticle(antiParticle);
     \endcode
   *
   * <h1>Accessing elements of ParticleList</h1>
   *
   * To loop over all particles (as well as their anti-particles) do the following
   *
     \code
     const unsigned int n = pList->getListSize();
     for (unsigned i = 0; i < n; i++) {
      const Particle* particle = pList->getParticle(i);

      // do something with the particle
      particle->....
     }
     \endcode
   *
   * If you would like to loop over the particles stored in a particular ParticleList _without_ including the anti-particles as well,
   * do the following (set the boolean parameter in the relevant functions to false)
     \code
     const unsigned int n = pList->getListSize(false);
     for (unsigned i = 0; i < n; i++) {
      const Particle* particle = pList->getParticle(i, false);

      // do something with the particle
      particle->....
     }
     \endcode
   *
   * You can also use C++11 range-based for loops to loop over all particles (and anti-particles):
   *
     \code
     for(const Particle& particle : *pList) {
        // do something with the particle
     }
   *
   * Remember to dereference the StoreObjPtr<ParticleList> before you can use it in the range based for loop.
   *
   * <h1>Remove Particles from ParticleList</h1>
   *
   * Particles can be removed in the following way (as above, this action will by default be applied to list of
   * particles as well as to list for anti-particles):
   *
     \code
      std::vector<unsigned int> toRemove;
      const unsigned int n = pList->getListSize();
      for (unsigned i = 0; i < n; i++) {
        const Particle* part = pList->getParticle(i);
        if (...particle should be removed...) toRemove.push_back(part->getArrayIndex());
      }

      pList->removeParticles(toRemove);
      \endcode
   */
  class ParticleList : public TObject {
  public:

    /** STL-like iterator over the particles (not Particle*). */
    typedef ArrayIterator<ParticleList, Particle> iterator;
    /** STL-like const_iterator over the particles (not Particle*). */
    typedef ArrayIterator<ParticleList, const Particle> const_iterator;

    /** Type of Particle (determines in which of the two internal lists the particle is stored). */
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
      m_thisListName(),
      m_antiListName(),
      m_antiList(nullptr)
    {
    }

    ~ParticleList();

    /**
     * Sets the PDG code and name of this ParticleList.
     *
     * At this point it is assumed that this is self-conjugated list (PDG code = anti-PDG code).
     * To bind the particle and anti-particle ParticleLists use bindAntiParticleList(ParticleList &antiList) method.
     *
     * @param pdg PDG code of particles to be hold in this list
     * @param name of the particle list
     * @param particleStoreName name of the Particle StoreArray
     */
    void initialize(int pdg, const std::string& name, const std::string& particleStoreName = "Particles");

    /**
     * Binds particle and anti-particle ParticleLists. After the lists are bound any action performed on ParticleList
     * for particles will be performed by default also on ParticleList for anti-particles.
     *
     * @param antiList - anti-particle ParticleList of this ParticleList
     * @param includingAntiList - the "anti"-anti-particle list has to be set also for the anti-particle list
     */
    void bindAntiParticleList(ParticleList& antiList, bool includingAntiList = true);

    /**
     * Sets Particle store array name to which particle list refers. By default this parameter is set to "Particles".
     *
     * @param name name of the Particle store array
     * @param forAntiParticle if true the name is set also for the anti-particle list
     */
    void setParticleCollectionName(const std::string& name, bool forAntiParticle = true);

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
     * @param includingAntiList include anti-particle list?
     */
    void addParticle(unsigned iparticle, int pdg, Particle::EFlavorType flavorType, bool includingAntiList = true);

    /**
     * Remove given elements from list
     *
     * @param toRemove vector of Particle array indices to be removed
     * @param removeFromAntiList flag indicating whether to remove particle also from anti-particle list (default is true)
     */
    void removeParticles(const std::vector<unsigned int>& toRemove, bool removeFromAntiList = true);

    /** Remove all elements from list, afterwards getListSize() will be 0.  */
    void clear(bool includingAntiList = true);

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
     * Possible options are:
     *
     * o) getList(ParticleList::c_FlavorSpecificParticle)
     *  - returns the vector of flavor-specific particles stored in this list
     *
     * o) getList(ParticleList::c_SelfConjugatedParticle)
     *  - returns the vector of self-conjugated particles stored in this list
     *
     * o) getList(ParticleList::c_FlavorSpecificParticle, true)
     *  - returns the vector of flavor-specific anti-particles stored in the anti-particle ParticleList of this list
     *
     * o) getList(ParticleList::c_SelfConjugatedParticle, true)
     *  - this is equivalent to getList(ParticleList::c_SelfConjugatedParticle)
     *
     * @param K ParticleType - Particle, SelfConjugatedParticle
     * @param forAntiParticle - whether the Particle or SelfConjugatedParticle should be returned for this (false) or anti-particle list (true)
     * @return const reference to vector of indices
     */
    const std::vector<int>& getList(EParticleType K, bool forAntiParticle = false) const;

    /**
     * Returns the name the anti-particle ParticleList.
     */
    std::string getAntiParticleListName() const { return m_antiListName; }

    /**
     * Returns the name this ParticleList.
     */
    std::string getParticleListName() const { return m_thisListName; }

    /**
     * Returns total number of particles in this list and anti list if requested
     *
     * @param includingAntiList
     * @return size of the this (+ anti-particle) list
     */
    unsigned getListSize(bool includingAntiList = true) const;

    /**
     * Returns i-th particle from the list and anti list if requested
     * @param i list index (i < getListSize())
     * @param includingAntiList consider anti-particle list as well?
     * @return pointer to Particle or NULL
     */
    Particle* getParticle(unsigned i, bool includingAntiList = true) const;

    /**
     * Returns the number of flavor-specific particles or self-conjugated particles in this list or its anti-particle list
     *
     * @param K ParticleType - Particle or SelfConjugatedParticle
     * @param forAntiParticle - whether the Particle or SelfConjugatedParticle should be returned for this (false) or anti-particle list (true)
     * @return number of particles or self-conjugated particles in the list or anti-particle list
     */
    unsigned getNParticlesOfType(EParticleType K, bool forAntiParticle = false) const
    {
      return getList(K, forAntiParticle).size();
    }

    /**
     * Returns true if and only if 'p' is already in this list.
     * Will check flavour-specific and self-conjugated list, and optionally (with includingAntiList=true) also the anti-particle list.
     */
    bool contains(const Particle* p, bool includingAntiList = true) const;

    /**
     * Prints the list
     */
    void print() const;

    /** Return a short summary of this object's contents in HTML format. */
    std::string getInfoHTML() const;

    /** Return iterator to first entry. */
    iterator begin() { return iterator(this, 0); }
    /** Return iterator to last entry +1. */
    iterator end() { return iterator(this, getListSize()); }

    /** Return const_iterator to first entry. */
    const_iterator begin() const { return const_iterator(this, 0); }
    /** Return const_iterator to last entry +1. */
    const_iterator end() const { return const_iterator(this, getListSize()); }

    /** Convenience function to get particle with index. **/
    Particle* operator[](int index) const {return getParticle(index);}

  private:
    /** Returns bound anti-particle list.
     *
     * Aborts if no list is present (might occur with I/O problems).
     * Please check m_antiListName is not empty() before calling this.
     */
    ParticleList& getAntiParticleList() const;

    int m_pdg;                   /**< PDG code of Particle */
    int m_pdgbar;                /**< PDG code of antiparticle */
    std::vector<int> m_fsList;   /**< list of 0-based indices of flavor-specific Particles (particles that have an anti-particle) */
    std::vector<int>
    m_scList;   /**< list of 0-based indices of self-conjugated Particles (particles that do not have an anti-particle) */

    std::string m_particleStore; /**< name of Particle store array */

    std::string m_thisListName;  /**< name of this ParticleList */
    std::string m_antiListName;  /**< name of ParticleList for anti-particles */

    /** keep anti-list around for performance. */
    mutable StoreObjPtr<ParticleList>* m_antiList; //! transient

    ClassDef(ParticleList, 3); /**< Class to hold a list of particles, anti-particles and self-conjugated particles. */

    friend class ParticleSubset;
  };

} // end namespace Belle2
