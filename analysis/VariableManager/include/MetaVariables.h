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
    Manager::FunctionPtr getExtraInfo(const std::vector<std::string>& arguments);

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
     * Returns function which returns the maximum q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr bestQrOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr QrOf(const std::vector<std::string>& arguments);

    /**
     * Returns 1 if the class track by particleName category has the same flavour as the MC target track,
     * 0 else; also if there is no target track. This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr IsRightClass(const std::vector<std::string>& arguments);

    /**
    * Checks if the given Particle was really a Particle from a B. 1.0 if true otherwise 0.0.
    * This is a Flavor Tagging specific variable!
    */
    Manager::FunctionPtr IsFromB(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns CMS momentum magnitude of the particle classified as target, i.e. that with the highest probability accessed via argument[1]
     * in the given list (argument[0]). This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr p_CMS(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns CMS recoiling mass of the Btag system against the target lepton, i.e. the particle that with the highest lepton probability accessed via argument[1]
     * in the given list (argument[0]). This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr recoilMassBtag(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns CMS momentum magnitude missing in Btag using as target hypothesis the particle with the highest lepton probability accessed via argument[1]
     * in the given list (argument[0]). This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr p_CMS_missing(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the cosine of the polar angle of the CMS momentum missing in Btag assuming a semileptonic decay. As target hypothesis is used the particle with the highest lepton probability accessed via argument[1]
     * in the given list (argument[0]). This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr particleCosTheta_CMS_missing(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the Energy in the hemisphere defined by the direction of the virtual W-Boson assuming a semileptonic decay. The target lepton is the particle with the highest probability accessed via argument[1]
     * in the given list (argument[0]). This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr E_W_90(const std::vector<std::string>& arguments);
  }
}
