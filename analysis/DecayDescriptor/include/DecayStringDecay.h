/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <boost/fusion/include/adapt_struct.hpp>
#include <analysis/DecayDescriptor/DecayString.h>
#include <analysis/DecayDescriptor/DecayStringParticle.h>

namespace Belle2 {
  /** Holds the information of a decay. Defined recursively,
  that is the daughters m_daughters can be a DecayString themselves.

  User documentation is located at analysis/doc/DecayDescriptor.rst
  Please modify in accordingly to introduced changes.*/
  struct DecayStringDecay {
    /** Mother particle. */
    DecayStringParticle m_mother;
    /** The arrow of the decay string. Different types of arrows are supported by the DecayDescriptor class. */
    std::string m_strArrow;
    /** The decay products. */
    std::vector<DecayString> m_daughters;
    /** The keyword list for custom MC Matching */
    std::vector<std::string> m_keywords;
  };
}

// introduce the DecayStringDecay struct to boost:fusion
// this enables direct parsing into the struct with the boost::spirit::qi package
// It is necessary to call this in the global scope
BOOST_FUSION_ADAPT_STRUCT(
  Belle2::DecayStringDecay,
  (Belle2::DecayStringParticle, m_mother)
  (std::string, m_strArrow)
  (std::vector<Belle2::DecayString>, m_daughters)
  (std::vector<std::string>, m_keywords)
)


