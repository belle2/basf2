/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>

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
    /** Particle property. Flags are defined in Particle::PropertyFlags */
    int m_properties;
    /** Label of this particle to distinguish e.g. different decay channels or selection criteria. */
    std::string m_strLabel;
    /** PDG code of the decaying particle. */
    int m_iPDGCode;
  public:
    /** Default ctor. */
    DecayDescriptorParticle();

    /** Want the default copy ctor. */
    DecayDescriptorParticle(const DecayDescriptorParticle&) = default;

    /** Want the default assignment operator */
    DecayDescriptorParticle& operator=(const DecayDescriptorParticle&) = default;

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
    /** return property of the particle. */
    int getProperty() const
    {
      return m_properties;
    }
    /** Is the particle unspecified? */
    bool isUnspecified() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsUnspecified);
    }
    /** Check if misID shall be ignored. */
    bool isIgnoreMisID() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreMisID);
    }
    /** Check if decayInFlight shall be ignored. */
    bool isIgnoreDecayInFlight() const
    {
      return (m_properties & Particle::PropertyFlags::c_IsIgnoreDecayInFlight);
    }
  };
}


