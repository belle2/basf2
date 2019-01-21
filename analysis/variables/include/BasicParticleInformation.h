/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Variables to return straightforward data about the particle (like the  *
 * number of daughters or particle type or source mdst object)            *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <vector>

namespace Belle2 {
  class Particle;

  namespace Variable {
    /**
     * return 1 if this particle was created from an ECLCluster, 0 otherwise
     */
    double particleIsFromECL(const Particle* part);

    /**
     * return 1 if this particle was created from a KLMCluster, 0 otherwise
     */
    double particleIsFromKLM(const Particle* part);

    /**
     * return 1 if this particle was created from a track, 0 otherwise
     */
    double particleIsFromTrack(const Particle* part);

    /**
     * return StoreArray index (0-based) of the MDST object from which the Particle was created
     */
    double particleMdstArrayIndex(const Particle* part);

    /**
     * return unique identifier for identification of Particles that are constructed from the same object in the detector (Track, energy deposit, ...)
     */
    double particleMdstSource(const Particle* part);
  }
}
