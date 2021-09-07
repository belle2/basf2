/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

#include <string>
#include <optional>

namespace Belle2 {
  class Particle;
  class DecayDescriptor;

  /** Class to help managing creation and adding to ParticleLists.
   *
   * There's a bit of management involved when trying to create a new particle list
   *
   * - the list needs to be registered in the datastore
   * - the conjugated list needs to be registered in the datastore unless the
   *   particle is self conjugated
   * - the particles array needs to be registered in the datastore
   * - in each event the lists need to be initialized
   * - when adding a particle it needs to be added to the particles array and then
   *   with the index it has in the array it is added to the particle list
   *
   * This class is managing all this in a clean interface
   *
   * 1. Add one instance as member to your class
   * 2. in the `initialize()` function call `registerList()` with the full list
   *    name (or the DecayDescriptor)
   * 3. in the beginning of the `event()` function call `create()`
   * 4. Fill particles with `add()` or `addParticle` (the second one is for the
   *    benefit of python as currently ROOT has problems with variadic template
   *    functions forwarding arguments)
   */
  class ParticleListHelper {
  public:
    /** Default constructor, does nothing */
    ParticleListHelper() = default;
    /** Construct and initialize right away. Useful in cases where you create
     * these objects directly in the `initialize()` function of a module
     */
    explicit ParticleListHelper(const std::string& listname, bool save = true): ParticleListHelper()
    {
      registerList(listname, save);
    }
    /** Register a list by name
     *
     * \warning should be called in initialize
     *
     * @param listname full list name in the form '{particlename}:{label}'
     * @param save if true the list will be saved in the output file
     */
    void registerList(const std::string& listname, bool save = true);
    /** Register a list using a decay descriptor
     *
     * \warning should be called in initialize
     *
     * @param decay decay descriptor to use for the list name
     * @param save if true the list will be saved in the output file
     */
    void registerList(const DecayDescriptor& decay, bool save = true);
    /** Return the pdg code of the main list. If it's not a self conjugated list
     * there is also a list with the negative pdg value */
    int getPDGCode() const { return m_pdg; }
    /** Return whether or not this list is self conjugated */
    bool isSelfConjugated() const { return not m_antiList.has_value(); }
    /** Create the list objects.
     * \warning This function should be called in **Every event** */
    void create();

    /** Add a particle to the list by forwarding all arguments to the constructor
     * of the Particle object: Any valid arguments to construct a Belle2::Particle work here.
     *
     * It will return a reference to the created particle for further modification.
     */
    template<class ...Args> Particle& add(Args&& ... args)
    {
      auto index = m_particles.getEntries();
      // cppcheck doesn't seem to understand the forwarding of arguments and
      // throws an erroneous warning about reassigning appendNew
      //
      // cppcheck-suppress redundantAssignment
      // cppcheck-suppress redundantInitialization
      auto particle = m_particles.appendNew(std::forward<Args>(args)...);
      m_list->addParticle(index, particle->getPDGCode(), particle->getFlavorType());
      return *particle;
    }

    /** Non-templated convenience function to add a particle from an existing particle object */
    Particle& addParticle(const Particle& particle) { return add(particle); }
    /** Non-templated convenience function to add a particle from simple four momentum
     * and an indicator whether it's particle or anti particle
     *
     * \warning particle/conjugates particle is based on what name was given when creating the list.
     *      If you created the list as 'anti-B0' than supplying 'conjugated=true' will create a 'B0'
     */
    Particle& addParticle(const ROOT::Math::PxPyPzEVector& momentum, bool conjugated = false)
    {
      return add(momentum, m_pdg * ((conjugated and not isSelfConjugated()) ? -1 : 1));
    }

  private:
    /** Store array for the particles */
    StoreArray<Particle> m_particles;
    /** Store object for the list */
    StoreObjPtr<ParticleList> m_list;
    /** Optional store object for the conjugated list if that exists */
    std::optional<StoreObjPtr<ParticleList>> m_antiList;
    /** PDG code of the list */
    int m_pdg;
  };
}
