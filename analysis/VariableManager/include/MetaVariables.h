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
     * Returns function which returns the remainder after division of value of variable by n
     * First argument in the argument vector must be the name of variable and the second argument is n
     */
    Manager::FunctionPtr modulo(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the true if the variable value is nan
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr isNAN(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the true if the variable value is infinity
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr isInfinity(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the absolute value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr abs(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the sine value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr sin(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the cosine value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr cos(const std::vector<std::string>& arguments);


    /**
     * Returns function which returns the variable of the nth daughter of the given particle
     * First argument in the argument vector must be an integer corresponding to the nth daughter.
     * Second argument the name of the variable.
     */
    Manager::FunctionPtr daughter(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1 if the combination of the particle the function acts upon
     * and a particle from the provided particle list is in the cut and 0 if it is not.
     */
    Manager::FunctionPtr veto(const std::vector<std::string>& arguments);

    /**
      * Returns -999 (delta function of NeuroBayes) instead of variable value if pid from given detector is missing.
      * First arguments must be TOP or ARICH
      * Second argument must be a valid variable name
      */
    Manager::FunctionPtr NBDeltaIfMissing(const std::vector<std::string>& arguments);

  }
}
