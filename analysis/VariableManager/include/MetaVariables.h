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
     * Returns function which returns the q*r value for a given list (argument[0]). The r value is stored via extraInfo
     * before and is accessed via argument[1]. This is a Flavor Tagging specific variable on event level!
     */
    Manager::FunctionPtr InputQrOf(const std::vector<std::string>& arguments);

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
     * Returns function which returns Returns 1.0 if the given Particle is classified as target, i.e. if it has the highest probability in particlelistName (argument[0]).
     * The probability is accessed via extraInfoName (argument[1]).
     * This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr hasHighestProbInCat(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns Returns the value of the highest target probability in particlelistName (argument[0]).
     * The probability is accessed via extraInfoName (argument[1]).
     * This is a Flavor Tagging specific variable!
     */
    Manager::FunctionPtr HighestProbInCat(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns a requested kinematical variable via argument[0] of the group of the semileptonic flavor tagging variables. The Possibilities are:
     * recoilMass: recoiling mass of the Btag system against the target particle in CMS
     * p_missing_CMS: CMS momentum magnitude missing in Btag using as target hypothesis the given particle
     * cosTheta_missing_CMS: cosine of the polar angle of the CMS momentum missing in Btag with respect to the momentum in CMs of the given Particle
     * EW90: Energy in the hemisphere defined by the direction of the virtual W-Boson assuming a semileptonic decay with the given particle as lepton
     * This are Flavor Tagging specific variables!
     */
    Manager::FunctionPtr SemiLeptonicVariables(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns a requested kinematical variable via argument[0] of the group of the flavor tagging variables corresponding to the FastSlowCorrelated categories.
     * The Possibilities are:
     * p_CMS_Fast: The CMS momentum of the fast track.
     * cosSlowFast: The cosine of the angle between the slow and the fast track.
     * cosTPTO_Fast: The cosine of the angle between the fast track and the thrust axis.
     * SlowFastHaveOpositeCharges: Returns 1 if the fast and slow target particles are oppositely charged, 0 else.
     * This are Flavor Tagging specific variables!
     */
    Manager::FunctionPtr FSCVariables(const std::vector<std::string>& arguments);

    /**
     * Returns the value of et, mm2, or one of the 16 KSFW moments, as requested by the user.
     */
    Manager::FunctionPtr KSFWVariables(const std::vector<std::string>& arguments);

  }
}
