/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * Returns the reduced Fox-Wolfram moment R2, calculated at the event level.
     */
    double R2EventLevel(const Particle* particle);

    /**
     * Returns the reduced Fox-Wolfram moment R2, calculated at the analysis level.
     */
    double R2(const Particle* particle);

    /**
     * Returns the magnitude of the B thrust axis.
     */
    double thrustBm(const Particle* particle);

    /**
     * Returns the magnitude of the ROE thrust axis.
     */
    double thrustOm(const Particle* particle);

    /**
     * Returns the cosine of angle between thrust axis of B and thrust axis of ROE.
     */
    double cosTBTO(const Particle* particle);

    /**
     * Returns the cosine of angle between thrust axis of B and z-axis.
     */
    double cosTBz(const Particle* particle);

    /**
     * Returns the value of et, mm2, or one of the 16 KSFW moments, as requested by the user.
     */
    Manager::FunctionPtr KSFWVariables(const std::vector<std::string>& arguments);

    /**
     * Returns the i-th Cleo Cone.
     */
    Manager::FunctionPtr CleoCones(const std::vector<std::string>& arguments);

    /**
     * Returns a function which gives the transformed network output C->C' via: C'=log((C-low)/(high-C)).
     * First argument in the argument vector must be the name of the extra info, i.e., the TMVA method (e.g., NeuroBayes, FastBDT, etc.).
     * Second (third) argument is the lower (upper) cut on the network output.
     */
    Manager::FunctionPtr transformedNetworkOutputNB(const std::vector<std::string>& arguments);

  }
}
