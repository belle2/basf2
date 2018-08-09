/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef DECAYDESCRIPTORPARTICLE_H
#define DECAYDESCRIPTORPARTICLE_H

#include <string>

namespace Belle2 {
  struct DecayStringParticle;

  /** Represents a particle in the DecayDescriptor.
  It is used for mother and daughter particles.

  User documentation is located at analysis/doc/DecayDescriptor.rst
  Please modify in accordingly to introduced changes.*/
  class DecayDescriptorParticle {
  private:
    /** evt.pdl name of the particle. */
    std::string m_strName;
    /** Is particle selected? */
    bool m_isSelected;
    /** Label of this particle to distinguish e.g. different decay channels or selection criteria. */
    std::string m_strLabel;
    /** PDG code of the decaying particle. */
    int m_iPDGCode;
  public:
    /** Default ctor. */
    DecayDescriptorParticle();
    /** Copy ctor. */
    DecayDescriptorParticle(const DecayDescriptorParticle& other);
    /** initialise member variables from std::string member variables contained in a DecayStringParticle struct. */
    bool init(const DecayStringParticle& p);
    /** evt.pdl name of the particle. */
    std::string getName() const
    {
      return m_strName;
    }
    /** returns the full name of the particle
     *  full_name = name:label
     */
    std::string getFullName() const
    {
      if (m_strLabel.empty())
        return m_strName;
      else
        return m_strName + ":" + m_strLabel;
    }
    /** Return the name from getName() without + - * or anti- */
    std::string getNameSimple() const;
    /** Is the particle selected in the decay string? */
    bool isSelected() const
    {
      return m_isSelected;
    }
    /** The label of this particle, "default" returned, when no label set. */
    std::string getLabel() const
    {
      return m_strLabel;
    }
    /** Return PDG code.*/
    int getPDGCode() const
    {
      return m_iPDGCode;
    }
  };
}

#endif // DECAYDESCRIPTORPARTICLE_H

