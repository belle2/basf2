/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef DECAYSTRING_H
#define DECAYSTRING_H
#include <analysis/DecayDescriptor/DecayStringParticle.h>
#include <boost/variant/recursive_variant.hpp>

namespace Belle2 {
  struct DecayStringDecay;
  /** The DecayStringElement can be either a DecayStringDecay or a vector of mother particles.

  User documentation is located at analysis/doc/DecayDescriptor.rst
  Please modify in accordingly to introduced changes.*/
  typedef boost::variant< boost::recursive_wrapper<DecayStringDecay>, DecayStringParticle > DecayString;
}

#endif // DECAYSTRING_H
