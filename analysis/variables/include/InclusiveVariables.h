/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * Number of photon daughters
     */
    int nDaughterPhotons(const Particle* particle);
    /**
     * Number of neutral hadron daughters
     */
    int nDaughterNeutralHadrons(const Particle* particle);
    /**
     * Number of charged daughters
     */
    Manager::FunctionPtr nDaughterCharged(const std::vector<std::string>& arguments);
    /**
     * PDG of the most common mother of daughters
     */
    int nDaughterNeutralHadrons(const Particle* particle);
    /**
     * PDG of the most common mother of daughters
     */
    int nCompositeDaughters(const Particle* particle);
    /**
     * Average variable values of daughters
     */
    Manager::FunctionPtr daughterAverageOf(const std::vector<std::string>& arguments);
  }
} // Belle2 namespace

