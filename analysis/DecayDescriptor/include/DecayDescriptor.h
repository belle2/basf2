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
#include <vector>
#include <utility>
#include <string>
#include <analysis/dataobjects/Particle.h>
#include <boost/ptr_container/ptr_vector.hpp>


namespace Belle2 {

  /** The DecayDescriptor stores information about
  how Particle objects are constructed. It contains
  the mother <-> daughter relations of the full decay
  chain including also daughters of daughters.
  Example: B -> D (D -> K pi) pi
  There is the possibility to select any particle of the decay
  chain. It is also possible to apply selection criteria
  using DecayCut objects. */
  class DecayDescriptor {
  private:
    /** Direct daughters of the decaying particle. */
    std::vector<DecayDescriptor> m_daughters;
    /** evt.pdl name of the decaying particle. */
    std::string m_strName;
    /** Tag of this particle to distinguish e.g. different decay channels or selection criteria. */
    std::string m_strTag;
    /** PDG code of the decaying particle. */
    int m_iPDGCode;
    /** Is this DecayDescriptor selected? */
    bool m_isSelected;
    /** Also consider charge conjugated mode? */
    bool m_isWithCC;
    /** Is this decay inclusive? */
    bool m_isInclusive;
    /** If this DecayDescriptor is attached as daughter
    of another DecayDescriptor, then it cannot be changed anymore! */
    bool m_isFixed;
    /** Is this the NULL object? */
    bool m_isNULL;
  public:
    /** Singleton object representing NULL. */
    const static DecayDescriptor& m_NULL;
    /** Dereference operator. */
    operator DecayDescriptor* () {
      return m_isNULL ? nullptr : this;
    }
    /** Defaut ctor. */
    DecayDescriptor();
    /** Copy ctor. */
    DecayDescriptor(const DecayDescriptor& other);
    /** Return the human readable name m_strName. */
    std::string getName() {
      return m_strName;
    };
    /** Return the name from getName() without + - * or anti- */
    std::string getNameSimple();
    /** Append a daughter to this particle. If the daughter decays further, then
    these decays have to be specified first because this sets daugher.m_isFixed = true. */
    void append(const DecayDescriptor& daughter);
    /** Initialise daughters from given string. */
    bool init(const std::string strDecayString);
    /** Select this DecayDescriptor. */
    void select(bool isSelected = true) {
      m_isSelected = isSelected;
    }
    /** Get a vector of pointers with selected daughters in the decay tree. */
    std::vector<const Particle*> getSelectionParticles(const Particle* particle);
    /** Return list of human readable names of selected particles.
    Example for the case that all particles are selected in B+ -> (anti-D0 -> K^- pi^+) pi^+:
    ["B", "D0", "D0_K", "D_pi", "pi"] */
    std::vector<std::string> getSelectionNames();
    /** Return PDG code.*/
    int getPDGCode() const {return m_iPDGCode;}
    /** return number of direct daughters. */
    int getNDaughters() const {return m_daughters.size();}
    /** return i-th daughter (0 based index). */
    const DecayDescriptor& getDaughter(int i) const {return (i < getNDaughters()) ? m_daughters[i] : DecayDescriptor::m_NULL;}
    /** Also consider charge conjugated mode? */
    bool isWithCC() const {return m_isWithCC;}
    /** Is the decay inclusive? */
    bool isInclusive() const {return m_isInclusive;}
  };

}

#endif // DECAYDESCRIPTOR_H
