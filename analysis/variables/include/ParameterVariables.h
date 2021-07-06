/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <vector>

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
     * If any particle or MCparticle is a nullptr, NaN is returned. If MC relations of any particle doesn't exist, -1.0 is returned.
     */
    double isAncestorOf(const Particle* part, const std::vector<double>& daughterIDs);

    /**
     * Returns a positive integer if an ancestor with the given PDG code is found, 0 otherwise.
     * The integer is the level where the ancestor was found, 1: first mother, 2: grandmother, etc.
     * Second argument is optional, 1 means that the sign of the PDG code is taken into account, default is 0.
     * If there is no MC relations found, -1 is returned. In case of nullptr particle, NaN is returned.
     */
    double hasAncestor(const Particle* part, const std::vector<double>& args);

    /**
     * return angle between the CMS momentum vector and the direction of the i-th daughter, both calculcated in the mother's rest frame
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
     * return angle between i-th daughter momentum vector and vector connecting production and decay vertex of i-th daughter
     */
    double pointingAngle(const Particle* particle, const std::vector<double>& daughters);

    /**
     * return azimuthal angle of i-th daughter in decay plane towards projection of particle momentum into decay plane
     */
    double azimuthalAngleInDecayPlane(const Particle* particle, const std::vector<double>& daughters);

  }
}
