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
     * return cosine of the angle between the mother momentum vector and the direction of the i-th daughter in the mother's rest frame
     */
    double particleDecayAngle(const Particle* particle, const std::vector<double>& daughters);

    /**
     * returns invariant mass of the daughters specified via the additional arguments
     */
    double daughterInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes);

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
