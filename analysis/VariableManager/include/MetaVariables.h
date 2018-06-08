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
     * If the extraInfo does not exist -999 is returned.
     */
    Manager::FunctionPtr extraInfo(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the value of the given variable for the given particle if its abs(pdgCode) agrees with the given one
     * First argument in the argument vector must be the name of variable.
     * Second argument in the argument vector must be an integers corresponding to a PDG code.
     */
    Manager::FunctionPtr varFor(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the number of particles in the given particle List.
     */
    Manager::FunctionPtr nParticlesInList(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1 if the given particle is a daughter of at least one of the particles of the
     * given particle Lists.
     */
    Manager::FunctionPtr isDaughterOfList(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1 if the given particle is a grand daughter of at least one of the particles of the
     * given particle Lists.
     */
    Manager::FunctionPtr isGrandDaughterOfList(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the product of a variable over all daughters of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr daughterProductOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the sum of a variable over all daughters of the given particle
     * The single argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr daughterSumOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the lowest value of a variable among all daughters of the given particle
     * The single argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr daughterLowest(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the highest value of a variable among all daughters of the given particle
     * The single argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr daughterHighest(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the given variable between the two given daughters
     * First two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     * Third argument the name of the variable.
     */
    Manager::FunctionPtr daughterDiffOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the normalized difference of the given variable between the two given daughters
     * First two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     * Third argument the name of the variable.
     */
    Manager::FunctionPtr daughterNormDiffOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the given variable between the particle and the its ith daughter.
     * First argument in the argument vector must be integer corresponding to the ith daughter.
     * Second argument the name of the variable.
     */
    Manager::FunctionPtr daughterMotherDiffOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the normalized difference of the given variable between the particle itself and
     * its ith daughter.
     * First argument in the argument vector must be integer corresponding to the ith daughter.
     * Second argument the name of the variable.
     */
    Manager::FunctionPtr daughterMotherNormDiffOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the angle between daughters:
     * If two indices given: returns the angle between the momenta of the two given daughters.
     * If three indices given: Variable returns the angle between the momentum of the third particle and a vector
     * which is the sum of the first two daughter momenta.
     * The arguments in the argument vector must be integers corresponding to the ith and jth (and kth) daughters.
     */
    Manager::FunctionPtr daughterAngleInBetween(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the invariant Mass m_ij=sqrt((pi + pj)^2) of the two given daughters
     * The two possible arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr daughterInvM(const std::vector<std::string>& arguments);

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
     * Returns function which returns the given value in the second argument if the given variable value is nan
     * First argument in the argument vector must be the name of variable. The second a number.
     */
    Manager::FunctionPtr ifNANgiveX(const std::vector<std::string>& arguments);

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
     * Returns a specific variable according to its rank in a particle list.
     * The rank is determined via BestCandidateSelection. BestCandidateSelection has to be used before.
     */
    Manager::FunctionPtr getVariableByRank(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1 if the combination of the particle the function acts upon
     * and a particle from the provided particle list is in the cut and 0 if it is not.
     */
    Manager::FunctionPtr veto(const std::vector<std::string>& arguments);

    /**
     * Returns the number of non-overlapping particles in the given particle lists
     * with respect to the particle the variable is applied on.
     * Arguments are the particle lists
     */
    Manager::FunctionPtr numberOfNonOverlappingParticles(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the total energy of particles in the given particle List.
    */
    Manager::FunctionPtr totalEnergyOfParticlesInList(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the total energy of particles in the given particle List in CMS.
    */
    Manager::FunctionPtr totalEnergyOfParticlesInListCMS(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the invariant mass of the combination of particles in the given particle Lists.
    */
    Manager::FunctionPtr invMassInLists(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the total ECL energy of particles in the given particle List.
    */
    Manager::FunctionPtr totalECLenergyOfParticlesInList(const std::vector<std::string>& arguments);

  }
}
