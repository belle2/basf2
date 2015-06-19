/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej, Anze Zupanc                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * returns information regarding the charm quark presence in the first level transition
     *
     * Example
     * If used on B mesons:
     * hasCharmedDaughter(-1) returns 1 if the process is a b->c or an anti-b->anti-c transition, 0 otherwise
     * hasCharmedDaughter(1) returns 1 if the process is a b->anti-c of an anti-b->c transition, 0 otherwise
     */
    double hasCharmedDaughter(const Particle* particle, const std::vector<double>& transition);

    /**
     * returns information regarding the charmonium state presence in the decay
     *
     * Example
     * If used on B mesons
     * hasCharmoniumDaughter returns 1 if the process is a b->c anti-c q or an anti-b->anti-c c anti-q transition, 0 otherwise
     */
    double hasCharmoniumDaughter(const Particle* particle);

    /**
     * returns information regarding daughters with a specific PDG code. It accepts the PDG code of interest, the number of particles of interest and an
     * option to take (-1) or not to take (1) the sign of the PDG code into account.
     *
     * Example
     * hasNDaughtersWithPDG(11,1,1) returns 1 if ONE daughter with the PDG code 11 or -11 (e- or e+) exists, 0 otherwise
     * hasNDaughtersWithPDG(-11,2,-1) returns 1 if TWO daughters with the PDG code -11 (e+) exist, 0 otherwise
     * hasNDaughtersWithPDG(11,0,1) returns 1 if there are NONE daughters with the PDG code 11 or -11 (e- or e+), 0 otherwise
     */
    double hasNDaughtersWithPDG(const Particle* particle, const std::vector<double>& args3);

    /**
     * returns information regarding daughters with a specific PDG code. It accepts the PDG code of interest and an
     * option to take (-1) or not to take (1) the sign of the PDG code into account.
     *
     * Example
     * hasDaughterWithPDG(11,1) returns 1 if there is at least one daughter with the PDG code 11 or -11 (e- or e+), 0 otherwise
     * hasDaughterWithPDG(-11,1) returns 1 if there is at least one daughter with the PDG code -11 (e+), 0 otherwise
     */
    double hasDaughterWithPDG(const Particle* particle, const std::vector<double>& args2);

    /**
     * returns information regarding daughters being real photons from EVTGEN or from PHOTOS, which are added later
     *
     * Example
     * hasRealPhotonDaughter returns 1 if at least one daughter of the particle is a real photon from EVTGEN and not from PHOTOS, 0 otherwise.
     */
    double hasRealPhotonDaughter(const Particle* particle);

  }
} // Belle2 namespace

