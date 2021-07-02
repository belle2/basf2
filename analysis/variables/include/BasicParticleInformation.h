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
     * return 1 if this particle was created from a V0, 0 otherwise
     */
    double particleIsFromV0(const Particle* part);

    /**
     * returns the mdst source used to create the particle
     */
    double particleSource(const Particle* part);

    /**
     * returns StoreArray index (0-based) of the MDST object from which the Particle was created
     */
    double particleMdstArrayIndex(const Particle* part);

    /**
     * returns unique identifier for identification of the final state particle
     */
    double uniqueParticleIdentifier(const Particle* part);

    /**
     * returns 1 if the particle is marked as an unspecified object (like B0 -> @Xsd e+ e-), 0 if not
     */
    double particleIsUnspecified(const Particle* part);

    /**
     * return prob(chi^2,ndf) of fit
     */
    double particlePvalue(const Particle* part);

    /**
     * return number of daughter particles
     */
    double particleNDaughters(const Particle* part);

    /**
     * return flavor type
     */
    double particleFlavorType(const Particle* part);

    /**
     * return charge
     */
    double particleCharge(const Particle* part);
  }
}
