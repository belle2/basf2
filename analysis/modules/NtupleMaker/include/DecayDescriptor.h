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

using namespace std;

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
    vector<DecayDescriptor> m_daughters;
    /** Human readable name of the decaying particle. */
    string m_strName;
    /** PDG code of the decaying particle. */
    int m_iPDGCode;
    /** Is this DecayDescriptor selected? */
    bool m_isSelected;
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
    /** Ctor from human readable particle name and PDG code of the decaying particle */
    DecayDescriptor(string strName, int iPDGCode);
    /** Return the human readable name m_strName. */
    string getName() {
      return m_strName;
    };
    /** Append a daughter to this particle. If the daughter decays further, then
    these decays have to be specified first because this sets daugher.m_isFixed = true. */
    void append(const DecayDescriptor& daughter);
    /** Initialise daughters from given string. */
    bool init(const string strDecayString);
    /** Select this DecayDescriptor. */
    void select(bool isSelected = true) {
      m_isSelected = isSelected;
    }
    /** Get a vector of pointers with selected daughters in the decay tree. */
    vector<const Particle*> getSelectionParticles(const Particle* particle);
    /** Return list of human readable names of selected particles.
    Example for the case that all particles are selected in B -> D (D -> K pi) pi:
    ["B", "D", "D_K", "D_pi", "pi"] */
    vector<string> getSelectionNames();
    /** Return PDG code.*/
    int getPDGCode() const {
      return m_iPDGCode;
    }
  };

}

#endif // DECAYDESCRIPTOR_H
