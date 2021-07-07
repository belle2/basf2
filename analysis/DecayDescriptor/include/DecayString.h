/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once
#include <analysis/DecayDescriptor/DecayStringParticle.h>
#include <boost/variant/recursive_variant.hpp>

namespace Belle2 {
  struct DecayStringDecay;
  /** The DecayStringElement can be either a DecayStringDecay or a vector of mother particles.

  User documentation is located at analysis/doc/DecayDescriptor.rst
  Please modify in accordingly to introduced changes.*/
  typedef boost::variant< boost::recursive_wrapper<DecayStringDecay>, DecayStringParticle > DecayString;
}

