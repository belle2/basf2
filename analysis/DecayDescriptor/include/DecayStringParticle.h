/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#ifndef DECAYSTRINGPARTICLE_H
#define DECAYSTRINGPARTICLE_H
#include <string>
#include <boost/fusion/include/adapt_struct.hpp>

namespace Belle2 {
  /** Holds the information of a particle in the decay string. Example: ^pi+:slow

  User documentation is located at analysis/doc/DecayDescriptor.rst
  Please modify in accordingly to introduced changes.*/
  struct DecayStringParticle {
    /** If the particle is selected by a preceding "^", this is stored here, if not, this is empty. */
    std::string m_strSelector;
    /** Particle name as it is defined in the evt.pdl file. */
    std::string m_strName;
    /** Optional label for this particle. */
    std::string m_strLabel;
  };
}

// introduce the DecayStringParticle struct to boost:fusion
// This enables direct parsing into the struct with the boost::spirit::qi package
// It is necessary to call this in the global scope
BOOST_FUSION_ADAPT_STRUCT(
  Belle2::DecayStringParticle,
  (std::string, m_strSelector)
  (std::string, m_strName)
  (std::string, m_strLabel)
)

#endif // DECAYSTRINGPARTICLE_H

