/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sviat Bilokin                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/dataobjects/Particle.h>
#include <analysis/VariableManager/Manager.h>

namespace Belle2 {

  namespace Variable {

    /**
     * Number of photon daughters
     */
    double nDaughterPhotons(const Particle* particle);
    /**
     * Number of neutral hadron daughters
     */
    double nDaughterNeutralHadrons(const Particle* particle);
    /**
     * Number of charged daughters
     */
    Manager::FunctionPtr nDaughterCharged(const std::vector<std::string>& arguments);
    /**
     * PDG of the most common mother of daughters
     */
    double nDaughterNeutralHadrons(const Particle* particle);
    /**
     * PDG of the most common mother of daughters
     */
    double nCompositeDaughters(const Particle* particle);
    /**
     * Average variable values of daughters
     */
    Manager::FunctionPtr daughterAverageOf(const std::vector<std::string>& arguments);
  }
} // Belle2 namespace

