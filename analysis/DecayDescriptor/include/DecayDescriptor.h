/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef DECAYDESCRIPTOR_H
#define DECAYDESCRIPTOR_H

#include <analysis/DecayDescriptor/DecayString.h>
#include <analysis/DecayDescriptor/DecayDescriptorParticle.h>

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
    /** Ignore radiated photons? */
    bool m_isIgnorePhotons;
    /** Ignore intermediate particles or resonances? */
    bool m_isIgnoreIntermediate;
    /** Is this decay inclusive? */
    bool m_isInclusive;
    /** Is this the NULL object? */
    bool m_isNULL;
    /** Internally called by match(Particle*) and match(MCParticle*) function. */
    template <class T>
    int match(const T* p, int iDaughter_p);
  public:
    /** Singleton object representing NULL. */
    const static DecayDescriptor& s_NULL;
    /** Dereference operator. */
    operator DecayDescriptor* ()
    {
      return m_isNULL ? nullptr : this;
    }
    /** Defaut ctor. */
    DecayDescriptor();
    /** Copy ctor. */
    DecayDescriptor(const DecayDescriptor& other);

    /** Initialise the DecayDescriptor from given string.
        Typically, the string is a parameter of an analysis module. */
    bool init(const std::string& str);

    /** Initialise the DecayDescriptor from a given DecayString.
    The DecayString struct is obtained from the parser called
    in the init(const std::string) function. */
    bool init(const DecayString& s);

    /** Checy if the DecayDescriptor matches with the given Particle.
    0 = no match
    1 = matches DecayDescriptor
    2 = matches charge conjugated DecayDescriptor
    3 = matches DeacyDescriptor AND charge conjugated DecayDescriptor
    -1, -2, -3 : same, but match is not unambigous. */
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
      return (i < getNDaughters()) ? &(m_daughters[i]) : NULL;
    }
    /** Check if additional photons shall be ignored. */
    bool isIgnorePhotons() const
    {
      return m_isIgnorePhotons;
    }
    /** Check if intermediate resonances/particles shall be ignored. */
    bool isIgnoreIntermediate() const
    {
      return m_isIgnoreIntermediate;
    }
    /** Is the decay inclusive? */
    bool isInclusive() const
    {
      return m_isInclusive;
    }

    /** Is the decay or the particle self conjugated */
    bool isSelfConjugated() const;

    /** Takes as input argument a (reconstructed) Particle, tries to match with
    this DecayDescriptorElement and returns true when matched. */
    bool getSelectionParticlesAndNames(const Particle* particle,
                                       std::vector<const Particle*>& selparticles,
                                       std::vector<std::string>& selnames);
  };

}

#endif // DECAYDESCRIPTOR_H
