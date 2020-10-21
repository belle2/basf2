/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014-2020 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck, Anze Zupanc, Sam Cunliffe,                  *
 *               Umberto Tamponi                                          *
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
     * If the extraInfo does not exist NaN is returned.
     */
    Manager::FunctionPtr extraInfo(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the value of the given variable for the given particle if its abs(pdgCode) agrees with the given one
     * First argument in the argument vector must be an integer corresponding to a PDG code.
     * Second argument in the argument vector must be the name of a variable.
     */
    Manager::FunctionPtr varFor(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the value of the given variable for the given particle if the MC particle related to it is primary, not virtual, and not initial.
     * If no MC particle is related to the given particle, or the MC particle is not primary, virtual, or initial, NaN will be returned.
     * Only one argument is allowed in the argument vector and it must be the name of a variable.
     */
    Manager::FunctionPtr varForMCGen(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the number of particles in the given particle List.
     */
    Manager::FunctionPtr nParticlesInList(const std::vector<std::string>& arguments);

    /**
     * Returns 1 if the particle is contained in the particle list
     */
    Manager::FunctionPtr isInList(const std::vector<std::string>& arguments);

    /**
     * Set certain bits in a variable to zero
     */
    Manager::FunctionPtr unmask(const std::vector<std::string>& arguments);


    /**
     * Returns 1 if the same mdst source object was used to create a particle in
     * the list (0 if not and -1 for non-mdst source based particles
     */
    Manager::FunctionPtr sourceObjectIsInList(const std::vector<std::string>& arguments);

    /**
     * Returns 1 if the same matched mcparticle is matched to a particle in
     * the list (0 if not). Also works for lists filled from mcparticles.
     */
    Manager::FunctionPtr mcParticleIsInMCList(const std::vector<std::string>& arguments);

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
     * Returns function which returns 1 if the given particle appears to be a daughter in the decay chain of given lists.
     */
    Manager::FunctionPtr isDescendantOfList(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns 1 if the given particle is linked to the same MC particle as any reconstructed daughter of the decay lists.
     * It makes only sense for lists created with fillParticleListFromMC function with addDaughters=True argument.
     */
    Manager::FunctionPtr isMCDescendantOfList(const std::vector<std::string>& arguments);

    /**
     * Returns a function which returns the the variable for the closest
     * particle in the provided particle list
     */
    Manager::FunctionPtr closestInList(const std::vector<std::string>& arguments);

    /**
     * Returns a function which returns the angle to the closest particle
     * in the provided particle list. See also closestInList.
     */
    Manager::FunctionPtr angleToClosestInList(const std::vector<std::string>& arguments);

    /**
     * Returns a function which returns the the variable for the most
     * back-to-back (closest to 180 degrees) particle in the provided particle list
     */
    Manager::FunctionPtr mostB2BInList(const std::vector<std::string>& arguments);

    /**
     * Returns a function which returns the angle to the most back-to-back
     * (closest to 180 degrees) particle in the provided particle list.
     * See also mostB2BInList.
     */
    Manager::FunctionPtr angleToMostB2BInList(const std::vector<std::string>& arguments);

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
     * Returns function which returns the difference of the given variable between the matched MC particles of two given daughters
     * First two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     * Third argument the name of the variable.
     */
    Manager::FunctionPtr mcDaughterDiffOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the given variable between the first daughters of the two given daughters
     * First two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     * Third argument the name of the variable.
     */
    Manager::FunctionPtr grandDaughterDiffOf(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable phi between the two given daughters
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr daughterDiffOfPhi(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable phi between the matched MC particles of the two given daughters
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr mcDaughterDiffOfPhi(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable phi between the first daughters of the two given daughters
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr grandDaughterDiffOfPhi(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable clusterPhi between the two given daughters
     * If (at least) one of the daughters does not have a (matched) ECLCluster, the function returns NaN
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr daughterDiffOfClusterPhi(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable clusterPhi between the first daughters of the two given daughters
     * If (at least) one of the daughters does not have a (matched) ECLCluster, the function returns NaN
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr grandDaughterDiffOfClusterPhi(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable phi between the two given daughters in the CMS frame
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr daughterDiffOfPhiCMS(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable phi between the matched MC particles of two given daughters in the CMS frame
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr mcDaughterDiffOfPhiCMS(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the difference of the angular variable clusterPhi between the two given daughters in the CMS frame
     * If (at least) one of the daughters does not have a (matched) ECLCluster, the function returns NaN
     * The two arguments in the argument vector must be integers corresponding to the ith and jth daughters.
     */
    Manager::FunctionPtr daughterDiffOfClusterPhiCMS(const std::vector<std::string>& arguments);

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
     * The arguments in the argument vector must be generalized daughter indices.
     */
    Manager::FunctionPtr daughterAngle(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the angle between MC particles matched to daughters:
     * If two indices given: returns the angle between the MC momenta of the two given daughters.
     * If three indices given: Variable returns the angle between the MC momentum of the third particle and a vector
     * which is the sum of the first two daughter MC momenta.
     * The arguments in the argument vector must be generalized daughter indices.
     */
    Manager::FunctionPtr mcDaughterAngle(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the angle between the granddaughter particle's
     * and the reverted particle's momentum vector in the daughter particle's rest frame.
     * Two arguments representing the indices of the daughter and granddaughter have to be provided as arguments.
     */
    Manager::FunctionPtr grandDaughterDecayAngle(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the angle between clusters associated to the two daughters.
     * If two indices given: returns the angle between the momenta of the clusters associated to the two given daughters.
     * If three indices given: returns the angle between the momentum of the third particle's cluster and a vector
     * which is the sum of the first two daughter's cluster momenta.
     * Returns nan if any of the daughters specified don't have an associated cluster.
     * The arguments in the argument vector must be integers corresponding to the ith and jth (and kth) daughters.
     */
    Manager::FunctionPtr daughterClusterAngleInBetween(const std::vector<std::string>& arguments);

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
     * Returns a function which returns the value of one of two variables of a particle,
     * depending on whether the particle passes the supplied cut. The first argument in the argument
     * vector must be a cut string, and the second and third arguments must be the name of the
     * variable to return if the particle does or does not pass the cut, respectively.
     */
    Manager::FunctionPtr conditionalVariableSelector(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the combined p-value of the given p-values
     * All arguments should be p-values in the range 0 to 1.
     */
    Manager::FunctionPtr pValueCombination(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the absolute value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr abs(const std::vector<std::string>& arguments);

    /**
     * Returns max value of two variables
     */
    Manager::FunctionPtr max(const std::vector<std::string>& arguments);

    /**
     * Returns min value of two variables
     */
    Manager::FunctionPtr min(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the sine value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr sin(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the arcsine value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr asin(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the cosine value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr cos(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the arccosine value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr acos(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the exponential value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr exp(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the natural log value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr log(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the log10 value of a variable of the given particle
     * First argument in the argument vector must be the name of variable
     */
    Manager::FunctionPtr log10(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the variable of the nth daughter of the given particle.
     * First argument in the argument vector must be an integer corresponding to the nth daughter.
     * Second argument the name of the variable.
     */
    Manager::FunctionPtr daughter(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the variable of the nth Monte Carlo daughter of the given particle.
     * First argument in the argument vector must be an integer corresponding to the nth Monte Carlo daughter.
     * As the order of the Monte Carlo daughter generally differs from the order of the reconstructed
     * daughters, one might need to write out additional information to identify the Monte Carlo Daugther
     * particles.
     * Second argument must be a valid variable.
     * If the particle is not matched to a MC particle or does not have a nth MC daughter NaN is returned.
     */
    Manager::FunctionPtr mcDaughter(const std::vector<std::string>& arguments);


    /**
     * Returns function which returns the variable for the Monte Carlo mother of the given particle.
     * The argument of the function must be a valid variable name.
     * If the particle is not matched with a Monte Carlo particle, or does not have a Monte Carlo
     * mother, NaN will be returned.
     */
    Manager::FunctionPtr mcMother(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the variable for the ith generator particle.
     * The arguments of the function must be
     *     argument 1: Index of the particle in the MCParticle Array
     *     argument 2: Valid basf2 function name of the function that shall be evaluated.
     * If the provided index goes beyond the length of the mcParticles array, NaN will be returned.
     */
    Manager::FunctionPtr genParticle(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns the variable for the generator level Upsilon(4S).
     * The argument of the function must be a valid basf2 function name of the function
     * that shall be evaluated.
     * If no generator level Upsilon(4S) exists for this event, NaN will be returned.
     */
    Manager::FunctionPtr genUpsilon4S(const std::vector<std::string>& arguments);

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
    * Returns function which returns the x-component of total momentum Px of particles in the given particle List.
    */
    Manager::FunctionPtr totalPxOfParticlesInList(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the y-component of total momentum Py of particles in the given particle List.
    */
    Manager::FunctionPtr totalPyOfParticlesInList(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the z-component of total momentum Pz of particles in the given particle List.
    */
    Manager::FunctionPtr totalPzOfParticlesInList(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the invariant mass of the combination of particles in the given particle Lists.
    */
    Manager::FunctionPtr invMassInLists(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the total ECL energy of particles in the given particle List.
    */
    Manager::FunctionPtr totalECLEnergyOfParticlesInList(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns maximum transverse momentum Pt in the given particle Lists.
    */
    Manager::FunctionPtr maxPtInList(const std::vector<std::string>& arguments);

    /**
     * Returns function which returns maximum opening angle in the given particle Lists.
     */
    Manager::FunctionPtr maxOpeningAngleInList(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns if at least one track is related to the cluster of the particle and this track satisfies the given condition.
    */
    Manager::FunctionPtr eclClusterTrackMatchedWithCondition(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the arithmetic mean of the given variable of the particles in the given particle list.
    */
    Manager::FunctionPtr averageValueInList(const std::vector<std::string>& arguments);

    /**
    * Returns function which returns the median value of the given variable of the particles in the given particle list.
    */
    Manager::FunctionPtr medianValueInList(const std::vector<std::string>& arguments);

    /**
    * Returns a function which returns the value of a variable obtained combining an arbitrary subset of particles in the decay tree, passed as
    * generalized indices. daughterCombination(M, 0, 3, 4) will return the invariant mass of the system made of the first, fourth and
    * fifth daugther of a particle.
    */
    Manager::FunctionPtr daughterCombination(const std::vector<std::string>& arguments);

    /**
     * Returns the value of the variable in the rest frame of the recoiling particle to the tag side B meson.
     * The variable should only be applied to an Upsilon(4S) list. E.g. ``useTagSideRecoilRestFrame(daughter(1, daughter(1, p)), 0)``
     * applied on a Upsilon(4S) list (``Upsilon(4S)->B+:tag B-:sig``) returns the momentum of the second daughter of the signal B
     * meson in the signal B meson rest frame."
     */
    Manager::FunctionPtr useTagSideRecoilRestFrame(const std::vector<std::string>& arguments);

    /**
    * Returns a  function that returns the value of a variable calculated using new mass assumptions for the daughters' masses.
    */
    Manager::FunctionPtr  useAlternativeDaughterHypothesis(const std::vector<std::string>& arguments);

    /**
     * Returns variable of particle's gen-level ancestor of given type
     */
    Manager::FunctionPtr varForFirstMCAncestorOfType(const std::vector<std::string>& arguments);

  }
}
