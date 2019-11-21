/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sviat Bilokin                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/variables/InclusiveVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <framework/gearbox/Const.h>

using namespace std;

namespace Belle2 {
  namespace Variable {

    double nDaughterPhotons(const Particle* particle)
    {
      int result = 0;
      auto fspDaughters = particle->getFinalStateDaughters();
      for (auto* daughter : fspDaughters) {
        if (abs(daughter->getPDGCode()) == Const::photon.getPDGCode()) {
          result++;
        }
      }
      return result;
    }
    // ---

    VARIABLE_GROUP("For fully-inclusive particles");

    REGISTER_VARIABLE("nDaughterPhotons",   nDaughterPhotons, "returns number of daugher photons.");
  }
}
