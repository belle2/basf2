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
     * returns true if this particle was created from an ECLCluster, false otherwise
     */
    bool particleIsFromECL(const Particle* part);

    /**
     * returns true if this particle was created from a KLMCluster, false otherwise
     */
    bool particleIsFromKLM(const Particle* part);

    /**
     * returns true if this particle was created from a track, false otherwise
     */
    bool particleIsFromTrack(const Particle* part);

    /**
     * returns true if this particle was created from a V0, false otherwise
     */
    bool particleIsFromV0(const Particle* part);

    /**
     * returns the mdst source used to create the particle
     */
    int particleSource(const Particle* part);

    /**
     * returns StoreArray index (0-based) of the MDST object from which the Particle was created
     */
    int particleMdstArrayIndex(const Particle* part);

    /**
     * returns unique identifier for identification of the final state particle
     */
    int uniqueParticleIdentifier(const Particle* part);

    /**
     * returns true if the particle is marked as an unspecified object (like B0 -> @Xsd e+ e-), false otherwise
     */
    bool particleIsUnspecified(const Particle* part);

    /**
     * return prob(chi^2,ndf) of fit
     */
    double particlePvalue(const Particle* part);

    /**
     * return number of daughter particles
     */
    int particleNDaughters(const Particle* part);

    /**
     * return flavor type
     */
    int particleFlavorType(const Particle* part);

    /**
     * return charge
     */
    double particleCharge(const Particle* part);
  }
}
