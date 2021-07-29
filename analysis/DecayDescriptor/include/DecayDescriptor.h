/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/DecayDescriptor/DecayString.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>
#include <analysis/dataobjects/Particle.h>

#include <vector>
#include <string>

namespace Belle2 {
  class MCParticle;
  class Particle;

  /** The DecayDescriptor stores information about
  a decay tree or parts of a decay tree. It contains
  the mother <-> daughter relations.

  User documentation is located at analysis/doc/DecayDescriptor.rst
  Please modify in accordingly to introduced changes.
  */
  class DecayDescriptor {
  private:
    /** Mother of the decay ('left side'). */
    DecayDescriptorParticle m_mother;
    /** ID of the Daughter Particle* matched to this DecayDescriptor.
    If this DecayDescriptor was not matched, the value is -1. */
    int m_iDaughter_p;
    /** Direct daughters of the decaying particle. */
    std::vector<DecayDescriptor> m_daughters;
    /** Particle property. Flags are defined in Particle::PropertyFlags */
    int m_properties;
    /** Is this the NULL object? */
    bool m_isNULL;
    /** Internally called by match(Particle*) and match(MCParticle*) function. */
    template <class T>
    int match(const T* p, int iDaughter_p);
    /** Collection of hierarchy paths of selected particles.
    Hierarchy path is vector of pairs of relative daughter numbers and particle names.
    For instance, in decay
    B+ -> [ D+ -> ^K+ pi0 ] pi0
    decay path of K+ is
    [(0, B), (0, D), (0 K)]
    Every selected particle has its own hierarchy path and
    they are stored as a vector in this variable:
    For the decayString
    B+ -> [ D+ -> ^K+ pi0 ] ^pi0
    m_hierarchy, once filled, is
    [[(0, B), (0, D), (0, K)],
    [(0, B), (1, pi0)]]
    */
    std::vector<std::vector<std::pair<int, std::string>>> m_hierarchy;

    /** Is this object initialized correctly? **/
    bool m_isInitOK;

  public:
    /** Singleton object representing NULL. */
    const static DecayDescriptor& s_NULL;

    /** Dereference operator. */
    operator DecayDescriptor* ()
    {
      return m_isNULL ? nullptr : this;
    }
    /** Default ctor. */
    DecayDescriptor();

    /** Want the default copy ctor. */
    DecayDescriptor(const DecayDescriptor&) = default;

    /** Want the default assignment operator */
    DecayDescriptor& operator=(const DecayDescriptor&) = default;

    /** Function to get hierarchy of selected particles  and their names (for python use) */
    std::vector<std::vector<std::pair<int, std::string>>>  getHierarchyOfSelected();

    /** Helper function to get hierarchy of selected particles  and their names. Called iteratively and get hierarchy path of a particle as an argument */
    std::vector<std::vector<std::pair<int, std::string>>>  getHierarchyOfSelected(const std::vector<std::pair<int, std::string>>&
        currentPath);

    /** Initialise the DecayDescriptor from given string.
        Typically, the string is a parameter of an analysis module. */
    bool init(const std::string& str);

    /** Initialise the DecayDescriptor from a given DecayString.
    The DecayString struct is obtained from the parser called
    in the init(const std::string) function. */
    bool init(const DecayString& s);

    /** Check if the DecayDescriptor matches with the given Particle.
    0 = no match
    1 = matches DecayDescriptor
    2 = matches charge conjugated DecayDescriptor
    3 = matches DeacyDescriptor AND charge conjugated DecayDescriptor
    -1, -2, -3 : same, but match is not unambiguous. */
    int match(const Particle* p) {return match<Particle>(p, -1);}

    /** See match(const Particle* p). */
    int match(const MCParticle* p) {return match<MCParticle>(p, -1);}

    /** Particle daughter ID set by previous call of match(const Particle*) function. */
    int getMatchedDaughter() {return m_iDaughter_p;}

    /** Reset results from previous call of the match() function. */
    void resetMatch();

    /** Get a vector of pointers with selected daughters in the decay tree. */
    std::vector<const Particle*> getSelectionParticles(const Particle* particle);
    /** Return list of human readable names of selected particles.
    Example for the case that all particles are selected in B+ -> (anti-D0 -> K^- pi^+) pi^+:
    ["B", "D0", "D0_K", "D_pi", "pi"] */
    std::vector<std::string> getSelectionNames();
    /** return mother. */
    const DecayDescriptorParticle* getMother() const
    {
      return &m_mother;
    }
    /** return number of direct daughters. */
    int getNDaughters() const
    {
      return m_daughters.size();
    }
    /** return i-th daughter (0 based index). */
    const DecayDescriptor* getDaughter(int i) const
    {
      return (i < getNDaughters()) ? &(m_daughters[i]) : nullptr;
    }
    /** return property of the particle. */
    int getProperty() const
    {
      return m_properties;
    }
    /** Check if additional radiated photons shall be ignored. */
    bool isIgnoreRadiatedPhotons() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreRadiatedPhotons) > 0;
    }
    /** Check if intermediate resonances/particles shall be ignored. */
    bool isIgnoreIntermediate() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreIntermediate) > 0;
    }
    /** Check if missing massive final state particles shall be ignored. */
    bool isIgnoreMassive() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreMassive) > 0;
    }
    /** Check if missing neutrinos shall be ignored. */
    bool isIgnoreNeutrino() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreNeutrino) > 0;
    }
    /** Check if missing gammas shall be ignored. */
    bool isIgnoreGamma() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreGamma) > 0;
    }
    /** Check if added Brems gammas shall be ignored. */
    bool isIgnoreBrems() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreBrems) > 0;
    }

    /** Is the decay or the particle self conjugated */
    bool isSelfConjugated() const;

    /** Check if the object initialized correctly. */
    bool isInitOK() const
    {
      return m_isInitOK;
    }

    /** Takes as input argument a (reconstructed) Particle, tries to match with
    this DecayDescriptorElement and returns true when matched. */
    bool getSelectionParticlesAndNames(const Particle* particle,
                                       std::vector<const Particle*>& selparticles,
                                       std::vector<std::string>& selnames);
  };

}

