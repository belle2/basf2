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
     * return i-th Cleo Cone
     */
    double CleoCones(const Particle* particle, const std::vector<double>& cone);

    /**
     * returns information regarding the charm quark presence in the decay
     *
     * hasCharmedDaughter(-1) returns 1 if there is a b->c or an anti-b->anti-c transition present, 0 otherwise
     * hasCharmedDaughter(1) returns 1 if there is a b->anti-c of an anti-b->c transition present, 0 otherwise
     */
    double hasCharmedDaughter(const Particle* particle, const std::vector<double>& transition);

  }
}
