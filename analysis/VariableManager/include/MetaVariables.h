/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>

#include <vector>
#include <string>

namespace Belle2 {

  namespace Variable {

    /**
     * Returns function which returns extra info of a given particle
     * First argument in the argument vector must be the name of the extra info
     */
    Manager::FunctionPtr extraInfo(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the product of a variable over all daughters of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr daughterProductOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the sum of a variable over all daughters of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr daughterSumOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the absolute value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr abs(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the variable of the nth daughter of the given particle
     * First argument in the argument vector must be an integer corresponding to the nth daughter.
     * Second argument the name of the variable.
     */
    Manager::FunctionPtr daughter(const std::vector<std::string>& arguments);

    /**
     * Returns -999 (delta function of NeuroBayes) instead of variable value if pid from given detector is missing.
     * First arguments must be TOP or ARICH
     * Second argument must be a valid variable name
     */
    Manager::FunctionPtr NBDeltaIfMissing(const std::vector<std::string>& arguments);


    /**
     * Returns the value of et, mm2, or one of the 16 KSFW moments, as requested by the user.
     */
    Manager::FunctionPtr KSFWVariables(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the transformed network output C->C' via: C'=log((C-low)/(high-C)).
     * First argument in the argument vector must be the name of the extra info, i.e., the TMVA method (e.g., NeuroBayes, FastBDT, etc.).
     * Second (third) argument is the lower (upper) cut on the network output.
     */
    Manager::FunctionPtr transformedNetworkOutputNB(const std::vector<std::string>& arguments);
  }
}
