/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam Cunliffe                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/variables/BasicParticleInformation.h>
#include <analysis/dataobjects/Particle.h>

namespace Belle2 {
  namespace Variable {

    double particleIsFromECL(const Particle* part)
    {
      return (part->getParticleType() == Particle::EParticleType::c_ECLCluster);
    }

    double particleIsFromKLM(const Particle* part)
    {
      return (part->getParticleType() == Particle::EParticleType::c_KLMCluster);
    }

    double particleIsFromTrack(const Particle* part)
    {
      return (part->getParticleType() == Particle::EParticleType::c_Track);
    }

    double particleMdstArrayIndex(const Particle* part)
    {
      return part->getMdstArrayIndex();
    }

    double particleMdstSource(const Particle* part)
    {
      return part->getMdstSource();
    }

    VARIABLE_GROUP("Basic particle information");
    REGISTER_VARIABLE("isFromECL", particleIsFromECL, "Returns 1.0 if this particle was created from an ECLCluster, 0 otherwise.");
    REGISTER_VARIABLE("isFromKLM", particleIsFromKLM, "Returns 1.0 if this particle was created from a KLMCluster, 0 otherwise.");
    REGISTER_VARIABLE("isFromTrack", particleIsFromTrack, "Returns 1.0 if this particle was created from a track, 0 otherwise.");
    REGISTER_VARIABLE("mdstIndex", particleMdstArrayIndex,
                      "StoreArray index(0 - based) of the MDST object from which the Particle was created");
    REGISTER_VARIABLE("mdstSource", particleMdstSource,
                      "mdstSource - unique identifier for identification of Particles that are constructed from the same object in the detector (Track, energy deposit, ...)");
  }
}
