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

#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns number of particles in the event with given pdg
     */
    double NumberOfMCParticlesInEvent(const Particle*, const std::vector<double>& pdg);

    /**
     * Returns a positive integer if daughter at position particle->daughter(i)->daughter(j)... is an ancestor of the related MC particle, 0 otherwise.
     * Positive integer represents the number of steps needed to get from final MC daughter to ancestor.
     * If any particle or MCparticle is a nullptr, -999 is returned. If MC relations of any particle doesn't exist, -1.0 is returned.
     */
    double isAncestorOf(const Particle* part, const std::vector<double>& daughterIDs);

    /**
     * Returns a positive integer if an ancestor with the given PDG code is found, 0 otherwise.
     * The integer is the level where the ancestor was found, 1: first mother, 2: grandmother, etc.
     * Second argument is optional, 1 means that the sign of the PDG code is taken into account, default is 0.
     * If there is no MC relations found, -1 is returned. In case of nullptr particle, -999 is returned.
     */
    double hasAncestor(const Particle* part, const std::vector<double>& args);

    /**
     * Check the PDG code of a particles n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc.
     */
    double genNthMotherPDG(const Particle* part, const std::vector<double>& daughterIDs);

    /**
     * Check the array index of a particle n-th MC mother particle by providing an argument. 0 is first mother, 1 is grandmother etc.
     */
    double genNthMotherIndex(const Particle* part, const std::vector<double>& daughterIDs);

    /**
     * return cosine of the angle between the mother momentum vector and the direction of the i-th daughter in the mother's rest frame
     */
    double particleDecayAngle(const Particle* particle, const std::vector<double>& daughters);

    /**
     * returns invariant mass of the daughters specified via the additional arguments
     */
    double daughterInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes);

    /**
     * returns true invariant mass of the daughters specified via the additional arguments
     */
    double daughterMCInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes);

    /**
     * returns mass difference between this Particle and one of the daughters specified via the additional argument
     */
    double massDifference(const Particle* particle, const std::vector<double>& daughter_index);

    /**
     * returns error estimate of the mass difference between this Particle and one of the daughters specified via the additional argument
     */
    double massDifferenceError(const Particle* particle, const std::vector<double>& daughter_index);

    /**
     * returns signed significance of the deviation from the nominal mass difference of this particle and its i-th daughter
     * [(massDiff - NOMINAL_MASS_DIFF)/ErrMassDiff]
     */
    double massDifferenceSignificance(const Particle* particle, const std::vector<double>& daughter_index);

    /**
     * return cosine of the angle between the i-th and j-th daughters
     */
    double particleDaughterAngle(const Particle* particle, const std::vector<double>& daughters);

    /**
     * return the d0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track
     */
    double v0DaughterD0(const Particle* particle, const std::vector<double>& daughterID);

    /**
     * return the difference between d0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track
     */
    double v0DaughterD0Diff(const Particle* particle);

    /**
     * return the z0 impact parameter of a V0's daughter with daughterID index with the V0 vertex point as a pivot for the track
     */
    double v0DaughterZ0(const Particle* particle, const std::vector<double>& daughterID);

    /**
     * return the difference between Z0 impact parameters of V0's daughters with the V0 vertex point as a pivot for the track
     */
    double v0DaughterZ0Diff(const Particle* particle);

  }
}
