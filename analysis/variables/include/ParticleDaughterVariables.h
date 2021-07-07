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
     * returns information regarding daughters being real photons from EVTGEN or from PHOTOS, which are added later
     *
     * Example
     * hasRealPhotonDaughter returns 1 if at least one daughter of the particle is a real photon from EVTGEN and not from PHOTOS, 0 otherwise.
     */
    double hasRealPhotonDaughter(const Particle* particle);

  }
} // Belle2 namespace

