/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns true if any L1 trigger bit is true
     */
    double L1Trigger(const Particle*);

    /**
     * returns trigger status for a given bit trigger bit
     */
    double L1TriggerBit(const Particle*, const std::vector<double>& bit);

    /**
     * returns trigger prescale for a given trigger bit
     */
    double L1TriggerBitPrescale(const Particle*, const std::vector<double>& bit);

  }
}
